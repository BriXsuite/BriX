#include "ReactorLite.h"

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

    LibraryReader(libraries[0], cyclus::Env::GetInstallPath() + "/share/brix/libraries/"\
                          + libraries[0], reactor_core_.library_);

    /// Library blending goes here, start by checking if libraries.size() > 1

    // Record relevant user data in reactor_core_
    reactor_core_.regions_ = regions;
    reactor_core_.thermal_pow_ = thermal_pow;
    reactor_core_.core_mass_ = core_mass;
    reactor_core_.target_BU_ = target_burnup;
    reactor_core_.target_CR_ = target_CR;
    reactor_core_.pnl = nonleakage;
    reactor_core_.fluence_timestep_ = fluence_timestep;
    reactor_core_.flux_mode_ = flux_mode;

    // Regions are populated based on reactor parameters
    RegionInfo region;

    for (int i = 0; i < regions; i++) {
        // ReactorLite has regions of equal mass
        region.mass_ = core_mass/regions;

        reactor_core_.region.push_back(region);
    }

    // Add structural material info
    StructReader(cyclus::Env::GetInstallPath() + "/share/brix/libraries/" + libraries[0],
                 reactor_core_.struct_prod_, reactor_core_.struct_dest_);

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
    std::cout << "Inventory: " << inventory.quantity() << std::endl;
    if (inventory.count() == 0) {return;}
    if (shutdown_) {return;}


    cyclus::Context* ctx = context();
    // Checks the state of the reactor and sets up the power output for the timestep
    if (outage_remaining_ > 1) {
        // Reactor still in outage
        outage_remaining_--;
        cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, 0);
        return;
    } else if (outage_remaining_ == 1) {
        // Reactor on last month of shutdown
        pow_per_time_ = thermal_pow * pow_frac_ * thermal_efficiency;
        outage_remaining_ = 0;
    } else {
        if (ctx->time() != cycle_end_) {
            cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, thermal_pow*thermal_efficiency);
            return;
        } else {
            if (pow_over_ + outage_time_ < 28.) {
                pow_frac_ = 1. - outage_time_/28.;
                pow_per_time_ = thermal_pow * pow_frac_ * thermal_efficiency;
            } else if (pow_over_ + outage_time_ >= 28. && pow_over_ + outage_time_ < 56.) {
                pow_frac_ = 2 - (pow_over_ + outage_time_)/28.;
                float x = pow_over_/28.;
                outage_remaining_ = 1;
                cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, thermal_pow*x*thermal_efficiency);
                return;
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

    reactor_core_.UpdateFractions(manifest);



/*
    //collapse iso's, read struct effects, reorder the fuel regions accordingly
    CoreBuilder();

    // record the burnup of the core before cycle begins
    float BU_prev = 0;
    float BU_next = 0;
    float delta_BU;

    for(int i = 0; i < reactor_core_.region.size(); i++){
        BU_prev += reactor_core_.region[i].return_BU();
    }
    BU_prev /= reactor_core_.region.size();
    //pass fuel bundles to burn-up calc

    if(CR_target < 0){
        burnupcalc(fuel_library_, flux_mode, DA_mode, burnupcalc_timestep);
    } else if (target_burnup > 0){
        if(refuels < regions){
            fuel_library_.target_BU = (float)(refuels+1.)*target_burnup/(float)regions;
        } else {
            fuel_library_.target_BU = target_burnup;
        }
        burnupcalc_CR(fuel_library_, flux_mode, DA_mode, burnupcalc_timestep);
    } else {
        burnupcalc(fuel_library_, flux_mode, DA_mode, burnupcalc_timestep);
    }

    // this is saved and may be used later for steady state calcs during blending
    ss_fluence = reactor_core_.region[regions-1].batch_fluence;

    //convert fuel bundle into materials
    for(int i = 0; i < reactor_core_.region.size(); i++){
        cyclus::CompMap out_comp;
        for(std::map<int, float>::iterator c = reactor_core_.region[i].comp.begin(); c != reactor_core_.region[i].comp.end(); ++c){
            if(c->second < 0){
            out_comp[pyne::nucname::zzaaam_to_id(c->first)] = 0;
        } else {
            out_comp[pyne::nucname::zzaaam_to_id(c->first)] = c->second;
        }
    }
    manifest[i]->Transmute(cyclus::Composition::CreateFromMass(out_comp));
    inventory.Push(manifest[i]);
  }
    // record burnup of the core after cycle ends
    for(int i = 0; i < reactor_core_.region.size(); i++){
        BU_next += reactor_core_.region[i].return_BU();
    }
    BU_next /= reactor_core_.region.size();

    delta_BU = (BU_next - BU_prev);
    if(delta_BU < 0){delta_BU = 0;}

    //cycle end update
    if(cycle_length > 0){
        cycle_end_ = ctx->time() + cycle_length;
        pow_over_ =  28. * ((delta_BU*core_mass/thermal_pow/28)-floor(delta_BU*core_mass/thermal_pow/28));
        //if the cycle length is less than 2 the fluence of regions will build up.
        if(cycle_end_ - ctx->time() < 1){
            std::cout << "---Warning, " << libraries[0] << " reactor cycle length too short. Do not trust results." << std::endl;
            std::cout << " --Cycle length will be manually increased for troubleshooting." << std::endl;
            cycle_end_ += 3; // this is done to help troubleshoot, results from runs where cycle length has to be adjusted shouldnt be trusted
        }

    } else {
        //std::cout << " DELTA BU "<<  delta_BU << "  BU_next: " << BU_next << "  BU_prev: " << BU_prev << std::endl;
        cycle_end_ = ctx->time() + floor(delta_BU*core_mass/generated_power/28.);
        pow_over_ =  28*((delta_BU*core_mass/generated_power/28)-floor(delta_BU*core_mass/generated_power/28));
    }


    //increments the number of times the reactor has been refueled.
    refuels += 1;

  //shutdown check
  if( (ctx->time() > start_time_ + reactor_life && record == true) || (refuels >= max_cycles) ){
    shutdown = true;
    std::cout << ctx->time() << " Agent " << id() << " shutdown after " << refuels << " cycles. Core CR: " << fuel_library_.CR << "  BU's: " << std::endl;
     for(int i = 0; i < reactor_core_.region.size(); i++){
        int ii;
        float burnup;

        burnup = reactor_core_.region[i].return_BU();
        std::cout << " Batch " << i+1 << ": "  << std::setprecision(4) << burnup << std::endl;
            // std::cout << " -> U235: " << reactor_core_.region[i].comp[922350] << " Fissile Pu: " << reactor_core_.region[0].comp[942390]
            // + reactor_core_.region[i].comp[942410] << " Total Pu: " << reactor_core_.region[i].comp[942380] + reactor_core_.region[i].comp[942390]
            // + reactor_core_.region[i].comp[942400] + reactor_core_.region[i].comp[942410] + reactor_core_.region[i].comp[942420] << std::endl;
        cyclus::toolkit::RecordTimeSeries("CR", this, fuel_library_.CR);
        cyclus::toolkit::RecordTimeSeries("BURNUP", this, burnup);
     }
    record = false;
    //std::cout << std::endl;
  }


  if(shutdown != true && record == true){
      std::cout << ctx->time() << " Agent " << id() << "  BU: "  << std::setprecision(4) << reactor_core_.region[0].discharge_BU << "  Batch CR: " <<
            reactor_core_.region[0].discharge_CR << " Cycle: " << cycle_end_ - ctx->time() << std::endl;
/*
        std::cout << " -> U235: " << reactor_core_.region[0].comp[922350] << " Fissile Pu: " << reactor_core_.region[0].comp[942390]
            + reactor_core_.region[0].comp[942410] << " Total Pu: " << reactor_core_.region[0].comp[942380] + reactor_core_.region[0].comp[942390]
            + reactor_core_.region[0].comp[942400] + reactor_core_.region[0].comp[942410] + reactor_core_.region[0].comp[942420] << std::endl;

        std::cout << " U238: " << reactor_core_.region[0].comp[922380] << " U236: " << reactor_core_.region[0].comp[922360]
            << " PU238: " << reactor_core_.region[0].comp[942380]  << " PU239: " << reactor_core_.region[0].comp[942390]
            << " PU240: " << reactor_core_.region[0].comp[942400]  << " PU241: " << reactor_core_.region[0].comp[942410] << std::endl
            << " AM241: " << reactor_core_.region[0].comp[952410]  << " AM243: " << reactor_core_.region[0].comp[952430]
            << " CS135: " << reactor_core_.region[0].comp[551350]  << " CS137: " << reactor_core_.region[0].comp[551370] << std::endl;
  }
    cyclus::toolkit::RecordTimeSeries("CR", this, fuel_library_.CR);
    cyclus::toolkit::RecordTimeSeries("BURNUP", this, reactor_core_.region[0].discharge_BU);
    cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, power_per_time);
*/

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



// WARNING! Do not change the following this function!!! This enables your
// archetype to be dynamically loaded and any alterations will cause your
// archetype to fail.
extern "C" cyclus::Agent* ConstructReactorLite(cyclus::Context* ctx) {
  return new ReactorLite(ctx);
}

}  // namespace ReactorLite
