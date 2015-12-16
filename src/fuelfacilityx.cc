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


// Accept fuel offered
void FuelFacilityX::AcceptMatlTrades(const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                        cyclus::Material::Ptr> >& responses) {

    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                            cyclus::Material::Ptr> >::const_iterator it;

    for (it = responses.begin(); it != responses.end(); ++it) {
        inventory.Push(it->second);
    }
}


// Offer materials
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> FuelFacilityX::GetMatlBids(
                            cyclus::CommodMap<cyclus::Material>::type& commod_requests) {

/*
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
    if (ctx->time() != cycle_end_ && storage_.count() == 0){
        return ports;
    }

    // If theres nothing to give dont offer anything
    if (storage_.count() == 0){return ports;}

    // Put everything in inventory to manifest
    std::vector<cyclus::Material::Ptr> manifest;
    //removing materials from storage
    int pop_number = 0;
    for(int i = 0; i < decay_times_.size(); i++){
        if(decay_times_[i] > decay_time_){
            pop_number++;
        }
    }
    manifest = cyclus::ResCast<Material>(storage_.PopN(storage_.count()));

    //Offering Bids
    std::vector<Request<Material>*>& requests = commod_requests[out_commod];
    std::vector<Request<Material>*>::iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
        Request<Material>* req = *it;
        if (req->commodity() == out_commod) {
            for(int i = 0; i < pop_number; i++) {
                Material::Ptr offer = Material::CreateUntracked(core_mass/regions, manifest[i]->comp());
                port->AddBid(req, offer, this);
            }
        }
    }
    storage_.PushAll(manifest);
    ports.insert(port);
    //std::cout << "end getmatlbids" << std::endl;
    return ports;
*/
    using cyclus::BidPortfolio;
    using cyclus::Material;
    std::set<BidPortfolio<Material>::Ptr> ports;
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
