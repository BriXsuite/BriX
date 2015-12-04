#include "ReactorX.h"
#include "structures.h"

namespace reactorx {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ReactorX::ReactorX(cyclus::Context* ctx) : cyclus::Facility(ctx) {
/*
    start_time_ = ctx->time();  // Start time is the current time
    cycle_end_ = start_time_;   // First cycle happens at the start time
*/
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string ReactorX::str() {
  return Facility::str();
}

//std::map<std::string, LibInfo> global_libs;

// First tick initializes the reactor. Not used later.
void ReactorX::Tick() {
    std::cout << "Hello world, I am ReactorX" << std::endl;


}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ReactorX::Tock() {



}

// The reactor requests the amount of regions it needs
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> ReactorX::GetMatlRequests() {
    /*
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
    */
}


// Accept fuel offered
void ReactorX::AcceptMatlTrades(const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                        cyclus::Material::Ptr> >& responses) {
/*
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
    */
}


// Offer materials
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> ReactorX::GetMatlBids(
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
}

// Discharging fuel from the reactor
void ReactorX::GetMatlTrades(const std::vector< cyclus::Trade<cyclus::Material> >& trades,
        std::vector<std::pair<cyclus::Trade<cyclus::Material>,cyclus::Material::Ptr> >& responses) {
    /*
    using cyclus::Material;
    using cyclus::Trade;
    cyclus::Context* ctx = context();
    //std::cout << "RX getTRADE START" << std::endl;
    std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
    for (it = trades.begin(); it != trades.end(); ++it) {
        cyclus::Material::Ptr discharge = cyclus::ResCast<Material>(storage_.Pop());
        discharge->Decay(ctx->time());
        decay_times_.erase(decay_times_.begin());
        responses.push_back(std::make_pair(*it, discharge));
    }
    //std::cout << "RX getTRADE end" << std::endl;
    */
}

// WARNING! Do not change the following this function!!! This enables your
// archetype to be dynamically loaded and any alterations will cause your
// archetype to fail.
extern "C" cyclus::Agent* ConstructReactorX(cyclus::Context* ctx) {
  return new ReactorX(ctx);
}

}  // namespace ReactorX
