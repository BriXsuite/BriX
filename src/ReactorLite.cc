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

    // First tick

    // Inform user reactor is starting up
    if(target_burnup == 0){
        std::cout << ctx->time()<< " New ReactorLite " << libraries[0] <<
        " reactor (ID:" << id() << ") starting up in forward mode." << std::endl;
    } else {
        std::cout << ctx->time() << " New ReactorLite " << libraries[0] <<
        " reactor (ID:" << id() << ") starting up - target burnup = " <<
        target_burnup << std::endl;
    }

    LibInfo temp_lib;
std::cout << "ablbala" << std::endl;
    LibraryReader(libraries[0], cyclus::Env::GetInstallPath() + "/share/brix/libraries/"\
                          + libraries[0], temp_lib);

    /// Library blending goes here, start by checking if libraries.size() > 1

    /// Records relevant user data in reactor_core_
    std::cout << "blbala" << std::endl;
    /*
    reactor_core_.regions_ = regions;
    reactor_core_.power_ = generated_power;
    reactor_core_.core_mass_ = core_mass;
    reactor_core_.target_BU_ = target_burnup;
    reactor_core_.target_CR_ = target_CR;
    reactor_core_.pnl = nonleakage;
*/
    // Regions are populated based on reactor parameters
    std::vector<RegionInfo> region;


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
