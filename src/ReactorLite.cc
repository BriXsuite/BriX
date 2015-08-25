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
    std::cout << "tick" << std::endl;
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

    LibInfo temp_lib;
    LibraryReader(libraries[0], cyclus::Env::GetInstallPath() + "/share/brix/libraries/"\
                          + libraries[0], temp_lib);

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

        ///TODO calculate and assign struct here

        reactor_core_.region.push_back(region);
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

}



// WARNING! Do not change the following this function!!! This enables your
// archetype to be dynamically loaded and any alterations will cause your
// archetype to fail.
extern "C" cyclus::Agent* ConstructReactorLite(cyclus::Context* ctx) {
  return new ReactorLite(ctx);
}

}  // namespace ReactorLite
