#include "ReactorLite.h"

namespace reactorlite {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ReactorLite::ReactorLite(cyclus::Context* ctx) : cyclus::Facility(ctx) {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string ReactorLite::str() {
  return Facility::str();
}

// First tick initializes the reactor. Not used later.
void ReactorLite::Tick() {
    // Skip if reactor shutdown or there is data
    if (shutdown == true | reactor_core_.region.size() != 0) {return;}



}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ReactorLite::Tock() {}



// WARNING! Do not change the following this function!!! This enables your
// archetype to be dynamically loaded and any alterations will cause your
// archetype to fail.
extern "C" cyclus::Agent* ConstructReactorLite(cyclus::Context* ctx) {
  return new ReactorLite(ctx);
}

}  // namespace ReactorLite
