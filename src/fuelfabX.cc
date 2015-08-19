#include "fuelfabX.h"

namespace fuelfabX {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    FuelfabX::FuelfabX(cyclus::Context* ctx)
        : cyclus::Facility(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    std::string FuelfabX::str() {
      return Facility::str();
    }
    /** CompOut prints out the ID and mass of each isotope in a cyclue material. */
    void CompOut(cyclus::Material::Ptr mat1){
        cyclus::CompMap comp;
        comp = mat1->comp()->mass(); //store the fractions of i'th batch in comp
        int comp_iso;
        cyclus::CompMap::iterator it;
        //each iso in comp
        for (it = comp.begin(); it != comp.end(); ++it){
            std::cout<<it->first<< " " << it->second << std::endl;
        }
    }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    void FuelfabX::Tick() {
        /** On the first tick the fuel fabrication facility will set up its inventories
        and give those inventories a capacity. Additionally there is some debugging code
        to check the size of each inventory inside of the fuel fabrication plant.*/
        if(inventory.size() == 0){
            cyclus::toolkit::ResourceBuff resource;
            for(int i = 0; i < in_commods.size(); i++){
                inventory.push_back(resource);
            }
        }//outputs whats inside the facility
        /*std::cout << "FuelFab " <<id() << " Inventory:" << std::endl;
        std::cout << "Fissile " << fissle_inv.quantity() << std::endl;
        std::cout << "Non Fissile " << non_fissle_inv.quantity() << std::endl;
        for(int i = 0; i < inventory.size(); i++){
            //sets max inventory at startup
            if(inventory[i].count() == 0){
                inventory[i].set_capacity(maximum_storage/inventory.size());
            }

            std::cout << "  " << i+1 << ": " << inventory[i].quantity() << std::endl;
            /*if(inventory[i].count() != 0){
                std::vector<cyclus::Material::Ptr> manifest;
                manifest = cyclus::ResCast<cyclus::Material>(inventory[i].PopN(inventory[i].count()));

                cyclus::CompMap comp;
                cyclus::CompMap::iterator it;

                comp = manifest[0]->comp()->mass();

                inventory[i].PushAll(manifest);
            }
        }*/
        //std::cout << "Fissile Inv: " << fissle_inv.quantity() <<std::endl;
    }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    void FuelfabX::Tock() {
        /** The fuel fabrication plant performs no functions on the tock */
    }

    std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> FuelfabX::GetMatlRequests() {
        /** During material requests the fuel fabrication plant  first requests new fuel from the fissile
        stream, then new fuel from the non-fissile stream, finally it requests fuel from the additional streams
        in incommods*/
        //std::cout << "ff GetMatreq" << std::endl;
        using cyclus::RequestPortfolio;
        using cyclus::Material;
        using cyclus::Composition;
        using cyclus::CompMap;
        using cyclus::CapacityConstraint;
        std::set<RequestPortfolio<Material>::Ptr> ports;
        cyclus::Context* ctx = context();
        CompMap cm;
        Material::Ptr target = Material::CreateUntracked(maximum_storage/inventory.size(), Composition::CreateFromAtom(cm));
        RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());

        double qty = fissle_inv.space();
        port->AddRequest(target, this, fissle_stream);
        CapacityConstraint<Material> cc(qty);
        port->AddConstraint(cc);
        ports.insert(port);

        RequestPortfolio<Material>::Ptr port2(new RequestPortfolio<Material>());
        qty = non_fissle_inv.space();
        port2->AddRequest(target, this, non_fissle_stream);
        CapacityConstraint<Material> cc2(qty);
        port2->AddConstraint(cc2);
        ports.insert(port2);

        std::map<std::string, double>::iterator i;
        int it = 0;
        for(i = in_commods.begin(); i != in_commods.end(); ++i){
            qty = inventory[it].space();
            RequestPortfolio<Material>::Ptr port3(new RequestPortfolio<Material>());
            port3->AddRequest(target, this, i->first);
            CapacityConstraint<Material> cc3(qty);
            port3->AddConstraint(cc3);
            ports.insert(port3);
            it++;
        }
        //std::cout << "ff GetMatreq end" << std::endl;
        return ports;
    }

    /** During get material bids the fuel fabrication plant recieves bid from all facilities requesting material from it.
    All Bright-lite reactors tell the fuel fabrication plant what type of fuel they need and how many batches of fuel,
    all other types of facilities are rejected*/
    std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>FuelfabX::GetMatlBids(cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
        using cyclus::BidPortfolio;
        using cyclus::CapacityConstraint;
        using cyclus::Converter;
        using cyclus::Material;
        using cyclus::Request;
        using reactor::ReactorFacility;
        cyclus::Context* ctx = context();
        std::set<BidPortfolio<Material>::Ptr> ports;
        //std::cout << "FF matbid" << std::endl;
        // respond to all requests of my commodity
        int inventory_test = 0;
        if(fissle_inv.count() > 0 && non_fissle_inv.count() > 0 ){
            inventory_test += 1;
        }
        for(int i = 0; i < inventory.size(); i++){
            if(inventory[i].count() > 0){
                inventory_test += 1;
            }
        }
        CapacityConstraint<Material> cc(1);
        if (inventory_test == 0){
            //std::cout << "FF matbid end" << std::endl;
            return ports;
        }


        std::vector<Request<Material>*>& requests = commod_requests[out_commod];
        std::vector<Request<Material>*>::iterator it;
        std::map<cyclus::Trader*, int> facility_request;
        Request<Material>* req;
        //Set up a map for facility requestor to number of requests

        BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
        for (it = requests.begin(); it != requests.end(); ++it) {
            Request<Material>* req = *it;
            facility_request[req->requester()] += 1;
        }
        //Iterate through requesters
        std::map<cyclus::Trader*, int>::iterator id;
        for(id = facility_request.begin(); id != facility_request.end(); ++id){
            double limit, nlimit;
            //Cast requester as a reactor if possible
            ReactorFacility* reactor = dynamic_cast<ReactorFacility*>(id->first);
            if (!reactor){
               throw cyclus::CastError("No reactor for fuelfab facility.");
            } else {

            }
        }
        //std::cout << "FF matbid end" << std::endl;
        return ports;
    }

    /** During accept material trades cyclus materials are added ot the fuel fabrication plants inventories*/
    void FuelfabX::AcceptMatlTrades(const std::vector< std::pair<cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >& responses) {
        //std::cout << "ff TRADE start" << std::endl;
        std::vector<std::pair<cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >::const_iterator it;
        for (it = responses.begin(); it != responses.end(); ++it) {
            std::map<std::string, double>::iterator i;
            double k = 0;
            for(i = in_commods.begin(); i != in_commods.end(); ++i){
                //std::cout << it->first.request->commodity() << " " << i->first << std::endl;
                if(it->first.request->commodity() == i->first){
                    inventory[k].Push(it->second);
                }
                k++;
            }
            if(it->first.request->commodity() == fissle_stream){
                fissle_inv.Push(it->second);
            }
            if(it->first.request->commodity() == non_fissle_stream){
                non_fissle_inv.Push(it->second);
            }
        }
        //std::cout << "ff TRADE end" << std::endl;
    }

    /** Get material trades uses the steady state enrichment determined by the reactor to discover
    the build new batches of material to send to the reactor.*/
    void FuelfabX::GetMatlTrades(const std::vector< cyclus::Trade<cyclus::Material> >& trades,
                                        std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                                        cyclus::Material::Ptr> >& responses) {
        using cyclus::Material;
        using cyclus::Trade;
        //std::cout << "ffgetTRADE start" << std::endl;
        std::map<cyclus::Trader*, int> facility_request;
        std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
        std::vector<cyclus::Material::Ptr> manifest;
        //Setting up comp map of requesters to number of requests
        for(it = trades.begin(); it != trades.end(); ++it){
            cyclus::Request<Material> req = *it->request;
            facility_request[req.requester()] += 1;
        }
        std::map<cyclus::Trader*, int>::iterator id;
        //iterate through requests
        for(id = facility_request.begin(); id != facility_request.end(); ++id){
            std::vector<double> mass_fracs;
            double limit, nlimit;
            //cast requester as reactor
            reactor::ReactorFacility* reactor = dynamic_cast<reactor::ReactorFacility*>(id->first);
            if (!reactor){
               throw cyclus::CastError("No reactor for fuelfab facility.");
            } else {

            }
        }
        //std::cout << "ff getTRADE end" << std::endl;
    }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    extern "C" cyclus::Agent* ConstructFuelfabX(cyclus::Context* ctx) {
        return new FuelfabX(ctx);
    }

}
