#include "fuelfacilityx.h"
#include "structures.h"

namespace fuelfacilityx {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FuelFacilityX::FuelFacilityX(cyclus::Context* ctx) : cyclus::Facility(ctx) {
/*
    start_time_ = ctx->time();  // Start time is the current time
    cycle_end_ = start_time_;   // First cycle happens at the start time
*/
    inventory.set_capacity(max_inv_size);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string FuelFacilityX::str() {
  return Facility::str();
}

//std::map<std::string, LibInfo> global_libs;

// First tick initializes the reactor. Not used later.
void FuelFacilityX::Tick() {
    std::cout << "Hello world, I am FuelFacilityX" << std::endl;
    std::cout << "  My inventory is: " << inventory.quantity() << std::endl;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FuelFacilityX::Tock() {


}

// The facility sends trade request to its source
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> FuelFacilityX::GetMatlRequests() {

    using cyclus::RequestPortfolio;
    using cyclus::Material;
    using cyclus::CapacityConstraint;
    using cyclus::Composition;
    using cyclus::CompMap;

    std::set<RequestPortfolio<Material>::Ptr> ports;

    // Don't request extra fuel
    if(inventory.quantity() >= tot_fuel_mass) {return ports;}

    CompMap cm;
    Material::Ptr target = Material::CreateUntracked(tot_fuel_mass, Composition::CreateFromAtom(cm));

    RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
    float qty = tot_fuel_mass - inventory.quantity();

    port->AddRequest(target, this, in_commod);

    CapacityConstraint<Material> cc(qty);

    port->AddConstraint(cc);
    ports.insert(port);

    return ports;
}


// Accept fuel offered from source
void FuelFacilityX::AcceptMatlTrades(const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                        cyclus::Material::Ptr> >& responses) {

    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                            cyclus::Material::Ptr> >::const_iterator it;

    for (it = responses.begin(); it != responses.end(); ++it) {
        inventory.Push(it->second);
    }
}


// Offer the fuel type this facility holds to ReactorX
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> FuelFacilityX::GetMatlBids(
                            cyclus::CommodMap<cyclus::Material>::type& commod_requests) {

    // Currently offers all available fuel

    using cyclus::BidPortfolio;
    using cyclus::CapacityConstraint;
    using cyclus::Converter;
    using cyclus::Material;
    using cyclus::Request;

    std::set<BidPortfolio<Material>::Ptr> ports;
    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

    // If theres nothing to give don't offer anything
    if(inventory.quantity() == 0){return ports;}

    // Put everything in inventory to manifest
    const unsigned int inv_count = inventory.count();

    std::vector<cyclus::Material::Ptr> offered_fuel;
    offered_fuel = cyclus::ResCast<Material>(inventory.PopN(inv_count));

    //Offering Bids
    std::vector<Request<Material>*>& requests = commod_requests[out_commod];
    std::vector<Request<Material>*>::iterator it;
    for(it = requests.begin(); it != requests.end(); ++it) {
        Request<Material>* req = *it;
        if(req->commodity() == out_commod) {
            for(int i = 0; i < inv_count; i++) {
                Material::Ptr offer = Material::CreateUntracked(max_inv_size, offered_fuel[i]->comp());
            }
        }
    }

    inventory.PushAll(offered_fuel);
    ports.insert(port);

    return ports;
}

// Sending fuel from the facility
void FuelFacilityX::GetMatlTrades(const std::vector< cyclus::Trade<cyclus::Material> >& trades,
        std::vector<std::pair<cyclus::Trade<cyclus::Material>,cyclus::Material::Ptr> >& responses) {
    /*
    using cyclus::Material;
    using cyclus::Trade;
    cyclus::Context* ctx = context();

    std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;

    for (it = trades.begin(); it != trades.end(); ++it) {
        cyclus::Material::Ptr fuel = cyclus::ResCast<Material>(storage_.Pop());
        fuel->Decay(ctx->time());
        decay_times_.erase(decay_times_.begin());
        responses.push_back(std::make_pair(*it, fuel));
    }
 */
}

// WARNING! Do not change the following this function!!! This enables your
// archetype to be dynamically loaded and any alterations will cause your
// archetype to fail.
extern "C" cyclus::Agent* ConstructFuelFacilityX(cyclus::Context* ctx) {
  return new FuelFacilityX(ctx);
}

}  // namespace FuelFacilityX
