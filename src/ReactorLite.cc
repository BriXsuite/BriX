#include "ReactorLite.h"
#include "structures.h"

namespace reactorlite {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ReactorLite::ReactorLite(cyclus::Context* ctx) : cyclus::Facility(ctx) {
    start_time_ = ctx->time();  // Start time is the current time
    cycle_end_ = start_time_;   // First cycle happens at the start time

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string ReactorLite::str() {
  return Facility::str();
}

//std::map<std::string, LibInfo> global_libs;

// First tick initializes the reactor. Not used later.
void ReactorLite::Tick() {
    cyclus::Context* ctx = context();
    // Return if this is not the first tick
    if (start_time_ != ctx->time()) {return;}

    // First tick, the rest of Tick() is like a constructor

    // Inform user reactor is starting up
    if (target_burnup == 0) {
        std::cout << ctx->time()<< " New ReactorLite " << libraries[0] <<
        " reactor (ID:" << id() << ") starting up in forward mode." << std::endl;
    } else {
        std::cout << ctx->time() << " New ReactorLite " << libraries[0] <<
        " reactor (ID:" << id() << ") starting up - target burnup = " <<
        target_burnup << std::endl;
    }
    bool lib_test = false;
    if(global_libs.count(libraries[0]) > 0){
        lib_test = true;
    }
    if(lib_test == false){
        std::cout << "bahs" << std::endl;
        LibraryReader(libraries[0], cyclus::Env::GetInstallPath() + "/share/brix/libraries/"\
                          + libraries[0], global_libs[libraries[0]]);
    }
    /// Library blending goes here, start by checking if libraries.size() > 1
    // Record relevant user data in reactor_core_
    reactor_core_.libraries_ = libraries;
    reactor_core_.regions_ = regions;
    reactor_core_.thermal_pow_ = thermal_pow;
    reactor_core_.core_mass_ = core_mass;
    reactor_core_.target_BU_ = target_burnup;
    reactor_core_.target_CR_ = target_CR;
    reactor_core_.pnl = nonleakage;
    reactor_core_.fluence_timestep_ = fluence_timestep;
    reactor_core_.base_flux_ = FluxFinder(cyclus::Env::GetInstallPath() +
                                          "/share/brix/libraries/" + libraries[0]);
    reactor_core_.flux_mode_ = flux_mode;
    reactor_core_.DA_mode_ = DA_mode;

    // Regions are populated based on reactor parameters
    RegionInfo region;

    for (int i = 0; i < regions; i++) {
        // ReactorLite has regions of equal mass
        region.mass_ = core_mass/regions;

        reactor_core_.region.push_back(region);
    }

    // Add structural material info
    if (struct_mode == 1) {
        StructReader(cyclus::Env::GetInstallPath() + "/share/brix/libraries/" + libraries[0],
                 reactor_core_.struct_prod_, reactor_core_.struct_dest_);
    }

    // Add spatial calculation parameters
    if (flux_mode == 2) {
        reactor_core_.spatial_.delta = spatial_delta;
        reactor_core_.spatial_.fuel_area = spatial_area;
        reactor_core_.spatial_.spatial_mod_thickness = spatial_mod_thickness;
        reactor_core_.spatial_.spatial_mod_Sig_f = spatial_mod_Sig_f;
        reactor_core_.spatial_.spatial_mod_Sig_tr = spatial_mod_Sig_tr;
        reactor_core_.spatial_.spatial_mod_Sig_a = spatial_mod_Sig_a;
        reactor_core_.spatial_.spatial_fuel_Sig_tr = spatial_fuel_Sig_tr;
    }

    // Add disadvantage factor calculation parameters
    if (DA_mode == 1) {
        reactor_core_.DA_.a = DA_a;
        reactor_core_.DA_.b = DA_b;
        reactor_core_.DA_.mod_Sig_a = DA_mod_Sig_a;
        reactor_core_.DA_.mod_Sig_s = DA_mod_Sig_s;
        reactor_core_.DA_.fuel_Sig_s = DA_fuel_Sig_s;
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ReactorLite::Tock() {
    if (inventory.count() == 0) {return;}
    if (shutdown_) {return;}

    cyclus::Context* ctx = context();
    // Checks the state of the reactor and sets up the power output for the timestep
    if (outage_remaining_ > 1) {
        // Reactor still in outage return zero power and reduce outage timer
        outage_remaining_--;
        cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, 0);
        return;
    } else if (outage_remaining_ == 1) {
        // Reactor on last month of shutdown
        pow_per_time_ = thermal_pow * pow_frac_ * thermal_efficiency;
        outage_remaining_ = 0;
    } else {
        // normal reactor operation
        if (ctx->time() != cycle_end_) {
            cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, thermal_pow*thermal_efficiency);
            return;
        } else {
            // if outage ends during same month as shutdown
            if (pow_over_ + outage_time_ < 28.) {
                pow_frac_ = 1. - outage_time_/28.;
                pow_per_time_ = thermal_pow * pow_frac_ * thermal_efficiency;
            // if outage ends the month following the shutdown
            } else if (pow_over_ + outage_time_ >= 28. && pow_over_ + outage_time_ < 56.) {
                pow_frac_ = 2 - (pow_over_ + outage_time_)/28.;
                float x = pow_over_/28.;
                outage_remaining_ = 1;
                cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, thermal_pow*x*thermal_efficiency);
                return;
            // if outage lasts more than the month after shutdown
            } else {
                outage_remaining_ = 2;
                while (pow_over_ + outage_time_ > outage_remaining_*28.) {
                    outage_remaining_++;
                }
                pow_frac_ = outage_remaining_-(pow_over_ + outage_time_)/28.;
                outage_remaining_--;
                float x = pow_over_/28.;
                cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, thermal_pow*x*thermal_efficiency);
                return;
            }
        }
    }

    // Pop materials out of inventory to create new regions and update fractions
    std::vector<cyclus::Material::Ptr> manifest;
    manifest = cyclus::ResCast<cyclus::Material>(inventory.PopN(inventory.count()));

    // Update the fractions in reactor_core_ with the popped manifest fractions
    reactor_core_.UpdateFractions(manifest);
    reactor_core_.BuildRegionIsos();
    ///TODO Call this only at startup (isos have to be built)
    reactor_core_.Reorder();
    // Record the burnup of the core before cycle begins
    const float BU_prev = reactor_core_.CalcBU();
    // Advance fluences accordingly
    BurnFuel(reactor_core_);
    // Determine change in core burnup in this step
    float delta_BU = reactor_core_.CalcBU() - BU_prev;
    if(delta_BU < 0) {delta_BU = 0;}

    // Update compositions
    reactor_core_.UpdateComp();

    // Convert into cyclus materials
    for(int reg_i = 0; reg_i < reactor_core_.region.size(); reg_i++){
        cyclus::CompMap out_comp;
        for(std::map<int, float>::iterator c = reactor_core_.region[reg_i].comp.begin();
                                            c != reactor_core_.region[reg_i].comp.end(); ++c){
            if(c->second < 0){
                out_comp[pyne::nucname::zzaaam_to_id(c->first)] = 0;
            } else {
                out_comp[pyne::nucname::zzaaam_to_id(c->first)] = c->second;
            }
        }
        manifest[reg_i]->Transmute(cyclus::Composition::CreateFromMass(out_comp));
        inventory.Push(manifest[reg_i]);
    }

    // Cycle length update
    if(cycle_length > 0){ // Cycle length override
        cycle_end_ = ctx->time() + cycle_length;
        pow_over_ =  28. * ((delta_BU*core_mass/thermal_pow/28)-floor(delta_BU*core_mass/thermal_pow/28));
    } else {
        cycle_end_ = ctx->time() + floor(delta_BU*core_mass/thermal_pow/28.);
        pow_over_ =  28*((delta_BU*core_mass/thermal_pow/28)-floor(delta_BU*core_mass/thermal_pow/28));

        // If the cycle length is less than 2 the fluence of regions may build up.
        if(cycle_end_ - ctx->time() < 1){
            std::cout << "---Warning, " << libraries[0] << " reactor cycle length too short. Do not trust results."
                    << std::endl << " --Cycle length will be manually increased for troubleshooting." << std::endl;
            cycle_end_ += 3; // This is done to help troubleshoot
        }
    }

    // Shutdown check
    if( (ctx->time() > start_time_ + reactor_life) || (refuels_ >= max_cycles) ) {
        shutdown_ = true;
        std::cout << ctx->time() << " Agent " << id() << " shutdown after " << refuels_ << " cycles. Core CR: " << reactor_core_.CR_ << "  BU's: " << std::endl;
         for(int i = 0; i < reactor_core_.region.size(); i++){
            const float burnup = reactor_core_.region[i].CalcBU();

            std::cout << " Batch " << i+1 << ": "  << std::setprecision(4) << burnup << std::endl;
                // std::cout << " -> U235: " << reactor_core_.region[i].comp[922350] << " Fissile Pu: " << reactor_core_.region[0].comp[942390]
                // + reactor_core_.region[i].comp[942410] << " Total Pu: " << reactor_core_.region[i].comp[942380] + reactor_core_.region[i].comp[942390]
                // + reactor_core_.region[i].comp[942400] + reactor_core_.region[i].comp[942410] + reactor_core_.region[i].comp[942420] << std::endl;
            cyclus::toolkit::RecordTimeSeries("CR", this, reactor_core_.CR_);
            cyclus::toolkit::RecordTimeSeries("BURNUP", this, burnup);
         }
        //std::cout << std::endl;
    }


    if(shutdown_ != true) {
        std::cout << ctx->time() << " Agent " << id() << "  BU: "  << std::setprecision(4)
                << reactor_core_.region[0].CalcBU() //TODO << "  Batch CR: " << reactor_core_.region[0].CR_
                << " Cycle: " << cycle_end_ - ctx->time() << std::endl;
/*
        std::cout << " -> U235: " << reactor_core_.region[0].comp[922350] << " Fissile Pu: " << reactor_core_.region[0].comp[942390]
            + reactor_core_.region[0].comp[942410] << " Total Pu: " << reactor_core_.region[0].comp[942380] + reactor_core_.region[0].comp[942390]
            + reactor_core_.region[0].comp[942400] + reactor_core_.region[0].comp[942410] + reactor_core_.region[0].comp[942420] << std::endl;

        std::cout << " U238: " << reactor_core_.region[0].comp[922380] << " U236: " << reactor_core_.region[0].comp[922360]
            << " PU238: " << reactor_core_.region[0].comp[942380]  << " PU239: " << reactor_core_.region[0].comp[942390]
            << " PU240: " << reactor_core_.region[0].comp[942400]  << " PU241: " << reactor_core_.region[0].comp[942410] << std::endl
            << " AM241: " << reactor_core_.region[0].comp[952410]  << " AM243: " << reactor_core_.region[0].comp[952430]
            << " CS135: " << reactor_core_.region[0].comp[551350]  << " CS137: " << reactor_core_.region[0].comp[551370] << std::endl;
  */}

    ///TODO move this to material exchange
    refuels_++;

    cyclus::toolkit::RecordTimeSeries("CR", this, reactor_core_.CR_);
    cyclus::toolkit::RecordTimeSeries("BURNUP", this, reactor_core_.region[0].CalcBU());
    cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this,  pow_per_time_);
}

// The reactor requests the amount of regions it needs
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> ReactorLite::GetMatlRequests() {
    using cyclus::RequestPortfolio;
    using cyclus::Material;
    using cyclus::CapacityConstraint;
    using cyclus::Composition;
    using cyclus::CompMap;

    std::set<RequestPortfolio<Material>::Ptr> ports;

    // Check refuel time
    cyclus::Context* ctx = context();
    if ((ctx->time() != cycle_end_ && inventory.count() != 0) || shutdown_ == true  ||
       outage_remaining_ > 0) {return ports;}

    CompMap cm;
    Material::Ptr target = Material::CreateUntracked(core_mass/regions, Composition::CreateFromAtom(cm));

    RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
    float qty;

    if (inventory.count() == 0) {
        // First loading, all regions need to be loaded with fuel

        if (target_burnup == 0) {
            // Forward mode

            for(int i = 0; i < regions; i++){
            // Handles if initial load regions are defined
            // Checks to see if there is a next in_commod to request, otherwise defualts to in_commods[0]
                if(in_commods.size() > i+1){
                    port->AddRequest(target, this, in_commods[i+1]);
                } else {
                    port->AddRequest(target, this, in_commods[0]);
                }
            }
        } else {
            // Blending mode

            for(int i = 0; i < regions; i++){
                port->AddRequest(target, this, in_commods[0]);
            }
        }
        qty = core_mass;
    } else {
        // One region refuel

        port->AddRequest(target, this, in_commods[0]);
        qty = core_mass/regions;
    }

    CapacityConstraint<Material> cc(qty);

    port->AddConstraint(cc);
    ports.insert(port);

    return ports;
}


// Accept fuel offered
void ReactorLite::AcceptMatlTrades(const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                        cyclus::Material::Ptr> >& responses) {

    if(shutdown_ != true){
        std::vector<std::pair<cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >::const_iterator it;
        cyclus::Composition::Ptr compost;

        if(target_burnup == 0){
            for (it = responses.begin(); it != responses.end(); ++it) {
                //std::cout << " incoming mass: " << it->second->quantity() << std::endl;
                inventory.Push(it->second);
                compost = it->second->comp();
                cyclus::CompMap cmap = compost->mass();
            }
        } else {
            //Operational reloading
            for (it = responses.begin(); it != responses.end(); ++it) {
                if(it->first.request->commodity() == in_commods[0]){
                    inventory.Push(it->second);
                }
            }
        }
    }
}


// Offer materials
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> ReactorLite::GetMatlBids(
                            cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
    // Offer either a single batch or a full core based on shutdown condition
    //std::cout << "RX GetMatBid" << std::endl;
    using cyclus::BidPortfolio;
    using cyclus::CapacityConstraint;
    using cyclus::Converter;
    using cyclus::Material;
    using cyclus::Request;

    cyclus::Context* ctx = context();
    std::set<BidPortfolio<Material>::Ptr> ports;
    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

    // If its not the end of a cycle dont get rid of your fuel
    if (ctx->time() != cycle_end_){
        return ports;
    }

    // If theres nothing to give dont offer anything
    if (inventory.count() == 0){return ports;}

    // Put everything in inventory to manifest
    std::vector<cyclus::Material::Ptr> manifest;
    manifest = cyclus::ResCast<Material>(inventory.PopN(inventory.count()));

    std::vector<Request<Material>*>& requests = commod_requests[out_commod];
    std::vector<Request<Material>*>::iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
        Request<Material>* req = *it;

        if (req->commodity() == out_commod) {
            if(shutdown_ == true) { // Offer all regions
                for(int i = 0; i < manifest.size(); i++) {
                    Material::Ptr offer = Material::CreateUntracked(core_mass/regions, manifest[i]->comp());
                    port->AddBid(req, offer, this);
                }
            } else { // Offer the oldest region
                //std::cout << " placing bid to discharge oldest fuel" << std::endl;
                Material::Ptr offer = Material::CreateUntracked(core_mass/regions, manifest[0]->comp());
                port->AddBid(req, offer, this);
                ///TODO read from real mass
                CapacityConstraint<Material> cc(core_mass/regions);
                port->AddConstraint(cc);

                if(std::abs(manifest[0]->quantity() - core_mass/regions) > 0.003){
                    std::cout << "-- Warning! Reactor " << id() << " is discharging a batch with mass "
                    << core_mass/regions - manifest[0]->quantity()
                    << " lower than input batch mass. Check upstream facility capacity." << std::endl;
                }
            }
        }
    }
    inventory.PushAll(manifest);

    ports.insert(port);
    //std::cout << "end getmatlbids" << std::endl;
    return ports;
}

// Discharging fuel from the reactor
void ReactorLite::GetMatlTrades(const std::vector< cyclus::Trade<cyclus::Material> >& trades,
        std::vector<std::pair<cyclus::Trade<cyclus::Material>,cyclus::Material::Ptr> >& responses) {
    using cyclus::Material;
    using cyclus::Trade;
    //std::cout << "RX getTRADE START" << std::endl;
    std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
    //Remove the core loading
    if(shutdown_ == true) {
        std::vector<cyclus::Material::Ptr> discharge = cyclus::ResCast<Material>(inventory.PopN(inventory.count()));
        reactor_core_.region.clear();
        int i = 0;
        for (it = trades.begin(); it != trades.end(); ++it) {
            responses.push_back(std::make_pair(*it, discharge[i]));
            i++;
        }
    } else {
        // Remove the last batch from the core.
        cyclus::Material::Ptr discharge = cyclus::ResCast<Material>(inventory.Pop());
        reactor_core_.region.erase(reactor_core_.region.begin());
        for (it = trades.begin(); it != trades.end(); ++it) {
            responses.push_back(std::make_pair(*it, discharge));
        }
    }
    //std::cout << "RX getTRADE end" << std::endl;
}

// WARNING! Do not change the following this function!!! This enables your
// archetype to be dynamically loaded and any alterations will cause your
// archetype to fail.
extern "C" cyclus::Agent* ConstructReactorLite(cyclus::Context* ctx) {
  return new ReactorLite(ctx);
}

}  // namespace ReactorLite
