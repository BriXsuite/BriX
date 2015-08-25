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
    reactor_core_.power_ = generated_power;
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
}

// The reactor requests the amount of batches it needs
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
            // Handles if initial load batches are defined
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
