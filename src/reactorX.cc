#include "reactorX.h"

namespace reactorX {

ReactorX::ReactorX(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {
      cycle_end_ = ctx->time() + 0;
      start_time_ = cycle_end_;
      shutdown = false;
      refuels = 0;
      record = true;
};

std::string ReactorX::str() {
  return Facility::str();
}


/** This function is used to measure the difference between the fuel being generated
for steady state calculations and the last fuel used in the reactor*/
bool stream_check(cyclus::Material::Ptr &mat1, cyclus::Material::Ptr &previous_mat){
    std::vector<double> diffs;
    bool check1;
    bool diff_check;
    std::map<int, double> comp_new = mat1->comp()->mass();
    std::map<int, double> comp_old = previous_mat->comp()->mass();
    cyclus::CompMap::iterator it;
    cyclus::CompMap::iterator id;
    for(it = comp_new.begin(); it != comp_new.end(); ++it){
        check1 = false;
        for(id = comp_old.begin(); id != comp_old.end(); ++id){
            if(it->first == id->first){
                diffs.push_back(std::pow(it->second - id->second, 2));
                check1 = true;
            }
        }
        if(check1 == false){diffs.push_back(std::pow(it->second, 2));}
    }
    double sum = 0;
    for(int i = 0; i < diffs.size(); i++){
        sum += diffs[i];
    }
    double rms = std::sqrt(sum/diffs.size());
    //std::cout << "RMS for reactor is: " << rms << std::endl;
    if (rms >= 0.001){return false;}
    else {return true;}
}

/** A function to output the isotopic composition of a cyclus material*/
void CompOutMat(cyclus::Material &mat1){
    cyclus::CompMap comp;
    comp = mat1.comp()->mass(); //store the fractions of i'th batch in comp
    int comp_iso;
    cyclus::CompMap::iterator it;
    //each iso in comp
    for (it = comp.begin(); it != comp.end(); ++it){
        //std::cout<<it->first<< " " << it->second << std::endl;
    }
}

/** A function to output the isotopic composition of a cyclus material pointer*/
void CompOut(cyclus::Material::Ptr &mat1){
    cyclus::CompMap comp;
    comp = mat1->comp()->mass(); //store the fractions of i'th batch in comp
    int comp_iso;
    cyclus::CompMap::iterator it;
    //each iso in comp
    for (it = comp.begin(); it != comp.end(); ++it){
        ///std::cout<<it->first<< " " << it->second << std::endl;
    }
}

/** During the first tick for the facilities the reactor libraries are set up and the
path of all important files is set up. This is general information for the reactor model
to operate.*/
void ReactorX::Tick() {
    //std::cout << "ReactorX inventory size: " << inventory.count() << std::endl;
    if(shutdown == true){return;}
    cyclus::Context* ctx = context();
    if(fuel_library_.name.size() == 0){
        cyclus::Context* ctx = context();
        if(target_burnup == 0){
            std::cout << ctx->time()<< " New " << libraries[0] << " reactor (ID:" << id() << ") starting up in forward mode." << std::endl;
        } else {
            std::cout << ctx->time() << " New " << libraries[0] << " reactor (ID:" << id() << ") starting up - target burnup = " << target_burnup << std::endl;
        }
        std::string manifest_file = cyclus::Env::GetInstallPath() + "/share/brightlite/" + \
                          libraries[0] + "/manifest.txt";
        //std::cout << manifest_file << std::endl;
        std::ifstream inf(cyclus::Env::GetInstallPath() + "/share/brightlite/" + \
                          libraries[0] + "/manifest.txt"); //opens manifest file
        std::string line;
        std::string iso_name;
        fuel_library_.name = libraries[0]; //for now only one entry in here
        // puts the name of every available iso in all_iso
        while(getline(inf, line)){
            std::istringstream iss(line);
            isoInformation iso; //creates temporary iso
            iss >> iso_name;
            iso.name = atoi(iso_name.c_str()); //adds name to iso
            iso.region = 0; //default region is the zeroth region
            iso.fraction = 0; //zero fraction
            fuel_library_.all_iso.push_back(iso); //adds this temp iso to fuel_library
        }
        if (libraries.size() < 2){
        //adds a built isoInfo for each isotope that can be in fuel, stores it in all_iso
            DataReader2(fuel_library_.name, fuel_library_.all_iso);
        } else {
            for(int i = 0; i < libraries.size(); i++){
                fuel_library_.interpol_libs.push_back(libraries[i]);
            }
            for(std::map<std::string, double>::iterator c = interpol_pairs.begin(); c != interpol_pairs.end(); ++c){
                interpol_pair pair;
                pair.metric = c -> first;
                pair.value = c -> second;
                fuel_library_.interpol_pairs.push_back(pair);
            }
            fuel_library_ = lib_interpol(fuel_library_);
        }
        //adds general info about the fuel in fuel_library_
        ///if theres value, dont update field
        fuel_library_.name = libraries[0];
        fuel_library_.base_flux = flux_finder(cyclus::Env::GetInstallPath() + "/share/brightlite/" + fuel_library_.name);
        fuel_library_.base_mass = core_mass;
        fuel_library_.base_power = generated_power;
        fuel_library_.pnl = nonleakage;
        fuel_library_.target_BU = target_burnup;
        fuel_library_.fuel_area = fuel_area;
        fuel_library_.cylindrical_delta = cylindrical_delta;
        fuel_library_.mod_Sig_a = mod_Sig_a;
        fuel_library_.mod_Sig_tr = mod_Sig_tr;
        fuel_library_.mod_Sig_f = mod_Sig_f;
        fuel_library_.mod_thickness = mod_thickness;
        fuel_library_.fuel_Sig_tr = fuel_Sig_tr;
        fuel_library_.disadv_a = disadv_a;
        fuel_library_.disadv_b = disadv_b;
        fuel_library_.disadv_mod_siga = disadv_mod_siga;
        fuel_library_.disadv_mod_sigs = disadv_mod_sigs;
        fuel_library_.disadv_fuel_sigs = disadv_fuel_sigs;
        fuel_library_.CR_lower = CR_lower;
        fuel_library_.CR_upper = CR_upper;
        fuel_library_.CR_target = CR_target;
        fuel_library_.SS_tolerance = SS_tolerance;

        batch_info empty_batch;
        //creates empty batch entries
        for(int i = 0; i < batches; i++){
            fuel_library_.batch.push_back(empty_batch);
            fuel_library_.batch[i].batch_fluence = 0; //fluence of the batch is set to zero
            fuel_library_.batch[i].DA = 1;
        }/*
        core_ = std::vector<fuelBundle>(batches); //core_ is size of batches, which will be changed
        for(int i = 0; i < core_.size(); i++){
          core_[i] = fuel_library_[i];
        }*/



        //read list of isotopes for conversion ratio calculation
   ///inputed as string, should be able to handle just numbers or letter number descrptn of isos
        for(int i = 0; i < CR_fissile.size(); i++){
            fuel_library_.CR_fissile.push_back(std::stoi(CR_fissile[i]));
        }
    }
    //std::cout << "end tick" << std::endl;
}

/** During the tock the reactor performs the burnup calculation and determines the availability
of the reactor for measure both power and outage periods. Additionally Bright-lite specific
cyclus tables are generated in this timestep. */
void ReactorX::Tock() {
    if(inventory.count() == 0){return;}
    if(shutdown == true){return;}

    cyclus::Context* ctx = context();
    // check the state of the reactor and sets up the power output of the reactor for the timestep
    if(outage_shutdown > 1){
        //reactor still in outage
        outage_shutdown--;
        cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, 0);
        return;
    } else if (outage_shutdown == 1){
        // reactor on last month of shutdown
        power_per_time = generated_power*p_frac*efficiency;
        outage_shutdown = 0;
    } else {
        if (ctx->time() != cycle_end_) {
            cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, generated_power*efficiency);
            return;
        } else {
            if(p_time + outage_time < 28.){
                p_frac = 1. - outage_time/28.;
                power_per_time = generated_power*p_frac*efficiency;
            } else if(p_time + outage_time >= 28. && p_time + outage_time < 56.){
                p_frac = 2-(p_time + outage_time)/28.;
                double x = p_time/28.;
                outage_shutdown = 1;
                cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, generated_power*x*efficiency);
                return;
            } else {
                outage_shutdown = 2;
                while (p_time + outage_time > outage_shutdown*28.) {
                    outage_shutdown++;
                }
                p_frac = outage_shutdown-(p_time + outage_time)/28.;
                outage_shutdown--;
                double x = p_time/28.;
                cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, generated_power*x*efficiency);
                return;
            }
        }
    }
    // Pop materials out of inventory to create new Bright-lite batches
    std::vector<cyclus::Material::Ptr> manifest;
    manifest = cyclus::ResCast<cyclus::Material>(inventory.PopN(inventory.count()));

    cyclus::CompMap comp;
    cyclus::CompMap::iterator it;
    if(manifest.size() > fuel_library_.batch.size()){
        //std::cout << "manifest: " << manifest.size() << "  fuel lib.batch: " << fuel_library_.batch.size() << std::endl;
        for(int i = 0; i < manifest.size() - fuel_library_.batch.size(); i++){
            batch_info temp_batch;
            temp_batch.batch_fluence = 0;
            temp_batch.DA = 1;
            fuel_library_.batch.push_back(temp_batch);
        }
    }
    //Put new isotope libraries into the bright-lite libraries for the new batch
    for(int i = 0; i < manifest.size(); i++){
    //build correct isoinfo and fraction of every isotope in each batch
    ///put the stuff in comp fraction to fuel_library_. .iso fraction using values in fuel_library_.all_iso
        comp = manifest[i]->comp()->mass(); //store the fractions of i'th batch in comp
        int comp_iso;
        //each iso in comp
        for (it = comp.begin(); it != comp.end(); ++it){
            comp_iso = pyne::nucname::zzaaam(it->first);
            //each iso in all_iso
            for(int j = 0; j < fuel_library_.all_iso.size(); j++){
                int fl_iso = fuel_library_.all_iso[j].name;
                if(fl_iso == comp_iso && fuel_library_.batch[i].batch_fluence == 0){
                    //std::cout << "i: " << i << "  " << fl_iso << "  " << comp_iso << "   "<<  it->second << std::endl;
                    isoInformation temp_iso;
                    temp_iso = fuel_library_.all_iso[j];
                    if(target_burnup == 0){
                        temp_iso.fraction = it->second;
                    } else {
                        temp_iso.fraction = it->second/(core_mass/batches);
                    }

                    fuel_library_.batch[i].iso.push_back(temp_iso);
                }
            }
        }
    }

    //collapse iso's, read struct effects, reorder the fuel batches accordingly
    CoreBuilder();

    // record the burnup of the core before cycle begins
    double BU_prev = 0;
    double BU_next = 0;
    double delta_BU;

    for(int i = 0; i < fuel_library_.batch.size(); i++){
        BU_prev += fuel_library_.batch[i].return_BU();
    }
    BU_prev /= fuel_library_.batch.size();
    //pass fuel bundles to burn-up calc

    if(CR_target < 0){
        burnupcalc(fuel_library_, flux_mode, DA_mode, burnupcalc_timestep);
    } else if (target_burnup > 0){
        if(refuels < batches){
            fuel_library_.target_BU = (double)(refuels+1.)*target_burnup/(double)batches;
        } else {
            fuel_library_.target_BU = target_burnup;
        }
        burnupcalc_CR(fuel_library_, flux_mode, DA_mode, burnupcalc_timestep);
    } else {
        burnupcalc(fuel_library_, flux_mode, DA_mode, burnupcalc_timestep);
    }

    // this is saved and may be used later for steady state calcs during blending
    ss_fluence = fuel_library_.batch[batches-1].batch_fluence;

    //convert fuel bundle into materials
    for(int i = 0; i < fuel_library_.batch.size(); i++){
        cyclus::CompMap out_comp;
        for(std::map<int, double>::iterator c = fuel_library_.batch[i].comp.begin(); c != fuel_library_.batch[i].comp.end(); ++c){
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
    for(int i = 0; i < fuel_library_.batch.size(); i++){
        BU_next += fuel_library_.batch[i].return_BU();
    }
    BU_next /= fuel_library_.batch.size();

    delta_BU = (BU_next - BU_prev);
    if(delta_BU < 0){delta_BU = 0;}

    //cycle end update
    if(cycle_length > 0){
        cycle_end_ = ctx->time() + cycle_length;
        p_time =  28. * ((delta_BU*core_mass/generated_power/28)-floor(delta_BU*core_mass/generated_power/28));
        //if the cycle length is less than 2 the fluence of batches will build up.
        if(cycle_end_ - ctx->time() < 1){
            std::cout << "---Warning, " << libraries[0] << " reactor cycle length too short. Do not trust results." << std::endl;
            std::cout << " --Cycle length will be manually increased for troubleshooting." << std::endl;
            cycle_end_ += 3; // this is done to help troubleshoot, results from runs where cycle length has to be adjusted shouldnt be trusted
        }

    } else {
        //std::cout << " DELTA BU "<<  delta_BU << "  BU_next: " << BU_next << "  BU_prev: " << BU_prev << std::endl;
        cycle_end_ = ctx->time() + floor(delta_BU*core_mass/generated_power/28.);
        p_time =  28*((delta_BU*core_mass/generated_power/28)-floor(delta_BU*core_mass/generated_power/28));
    }


    //increments the number of times the reactor has been refueled.
    refuels += 1;

  //shutdown check
  if( (ctx->time() > start_time_ + reactor_life && record == true) || (refuels >= max_cycles) ){
    shutdown = true;
    std::cout << ctx->time() << " Agent " << id() << " shutdown after " << refuels << " cycles. Core CR: " << fuel_library_.CR << "  BU's: " << std::endl;
     for(int i = 0; i < fuel_library_.batch.size(); i++){
        int ii;
        double burnup;

        burnup = fuel_library_.batch[i].return_BU();
        std::cout << " Batch " << i+1 << ": "  << std::setprecision(4) << burnup << std::endl;
            // std::cout << " -> U235: " << fuel_library_.batch[i].comp[922350] << " Fissile Pu: " << fuel_library_.batch[0].comp[942390]
            // + fuel_library_.batch[i].comp[942410] << " Total Pu: " << fuel_library_.batch[i].comp[942380] + fuel_library_.batch[i].comp[942390]
            // + fuel_library_.batch[i].comp[942400] + fuel_library_.batch[i].comp[942410] + fuel_library_.batch[i].comp[942420] << std::endl;
        cyclus::toolkit::RecordTimeSeries("CR", this, fuel_library_.CR);
        cyclus::toolkit::RecordTimeSeries("BURNUP", this, burnup);
     }
    record = false;
    //std::cout << std::endl;
  }


  if(shutdown != true && record == true){
      std::cout << ctx->time() << " Agent " << id() << "  BU: "  << std::setprecision(4) << fuel_library_.batch[0].discharge_BU << "  Batch CR: " <<
            fuel_library_.batch[0].discharge_CR << " Cycle: " << cycle_end_ - ctx->time() << std::endl;
/*
        std::cout << " -> U235: " << fuel_library_.batch[0].comp[922350] << " Fissile Pu: " << fuel_library_.batch[0].comp[942390]
            + fuel_library_.batch[0].comp[942410] << " Total Pu: " << fuel_library_.batch[0].comp[942380] + fuel_library_.batch[0].comp[942390]
            + fuel_library_.batch[0].comp[942400] + fuel_library_.batch[0].comp[942410] + fuel_library_.batch[0].comp[942420] << std::endl;

        std::cout << " U238: " << fuel_library_.batch[0].comp[922380] << " U236: " << fuel_library_.batch[0].comp[922360]
            << " PU238: " << fuel_library_.batch[0].comp[942380]  << " PU239: " << fuel_library_.batch[0].comp[942390]
            << " PU240: " << fuel_library_.batch[0].comp[942400]  << " PU241: " << fuel_library_.batch[0].comp[942410] << std::endl
            << " AM241: " << fuel_library_.batch[0].comp[952410]  << " AM243: " << fuel_library_.batch[0].comp[952430]
            << " CS135: " << fuel_library_.batch[0].comp[551350]  << " CS137: " << fuel_library_.batch[0].comp[551370] << std::endl;
*/  }
    cyclus::toolkit::RecordTimeSeries("CR", this, fuel_library_.CR);
    cyclus::toolkit::RecordTimeSeries("BURNUP", this, fuel_library_.batch[0].discharge_BU);
    cyclus::toolkit::RecordTimeSeries<cyclus::toolkit::POWER>(this, power_per_time);
}

/** The reactor requests the amount of batches it needs*/
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> ReactorX::GetMatlRequests() {
    //std::cout << "Getmatlrequests begin" << std::endl;
    using cyclus::RequestPortfolio;
    using cyclus::Material;
    using cyclus::Composition;
    using cyclus::CompMap;
    using cyclus::CapacityConstraint;
    std::set<RequestPortfolio<Material>::Ptr> ports;
    cyclus::Context* ctx = context();
    if (ctx->time() != cycle_end_ && inventory.count() != 0 || shutdown == true  || outage_shutdown > 0){return ports;}

    CompMap cm;
    Material::Ptr target = Material::CreateUntracked(core_mass/batches, Composition::CreateFromAtom(cm));

    RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
    double qty;

    if(inventory.count() == 0){
        if(target_burnup == 0){
            for(int i = 0; i < batches; i++){
            //checks to see if there is a next in_commod to request, otherwise puts the first commod request
                if(in_commods.size() > i+1){
                    port->AddRequest(target, this, in_commods[i+1]);
                } else {
                    port->AddRequest(target, this, in_commods[0]);
                }
            }
        } else {
            for(int i = 0; i < batches; i++){
                port->AddRequest(target, this, in_commods[0]);
            }
        }

        qty = core_mass;
    } else {
        port->AddRequest(target, this, in_commods[0]);
        qty = core_mass/batches;
    }
    CapacityConstraint<Material> cc(qty);

    port->AddConstraint(cc);
    ports.insert(port);
    //std::cout << "end getmatlrequests" << std::endl;
    return ports;
}

/** The reactor will offer either a single batch for a full core load depending on shut down
condition.*/
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> ReactorX::GetMatlBids(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  //std::cout << "RX GetMatBid" << std::endl;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;
  cyclus::Context* ctx = context();
  std::set<BidPortfolio<Material>::Ptr> ports;

  //if its not the end of a cycle dont get rid of your fuel
  if (ctx->time() != cycle_end_){
    return ports;
  }

  //if theres nothing to give dont offer anything..
  if (inventory.count() == 0){return ports;}


  std::vector<cyclus::Material::Ptr> manifest;
  manifest = cyclus::ResCast<Material>(inventory.PopN(inventory.count()));
  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  std::vector<Request<Material>*>& requests = commod_requests[out_commod];
  std::vector<Request<Material>*>::iterator it;
  for (it = requests.begin(); it != requests.end(); ++it) {
    Request<Material>* req = *it;
    //std::cout << " request quant: " << req->quantity() << std::endl;
    if (req->commodity() == out_commod) {
        if(shutdown == true){
            for(int i = 0; i < manifest.size(); i++){
                Material::Ptr offer = Material::CreateUntracked(core_mass/batches, manifest[i]->comp());
                port->AddBid(req, offer, this);
            }
        } else{
            //std::cout << " placing bid to discharge oldest fuel" << std::endl;
            Material::Ptr offer = Material::CreateUntracked(core_mass/batches, manifest[0]->comp());
            port->AddBid(req, offer, this);
            CapacityConstraint<Material> cc(core_mass/batches);
            port->AddConstraint(cc);

            if(std::abs(manifest[0]->quantity() - core_mass/batches) > 0.003){
                std::cout << "-- Warning! Reactor " << id() << " is discharging a batch with mass "
                << core_mass/batches - manifest[0]->quantity() << " lower than input batch mass. Check upstream facility capacity." << std::endl;
            }
        }
    }
  }
  inventory.PushAll(manifest);

  /*if(shutdown != true){
    CapacityConstraint<Material> cc(core_mass/batches);
    port->AddConstraint(cc);
  }*/

  ports.insert(port);
  //std::cout << "end getmatlbids" << std::endl;
  return ports;
}

/** Accepts fuel offered to it*/
void ReactorX::AcceptMatlTrades(const std::vector< std::pair<cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >& responses) {
    //std::cout << "RX TRADE START" << std::endl;
    if(shutdown != true){
        std::vector<std::pair<cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >::const_iterator it;
        cyclus::Composition::Ptr compost;

        if(target_burnup == 0){
            for (it = responses.begin(); it != responses.end(); ++it) {
                //std::cout << " incoming mass: " << it->second->quantity() << std::endl;
                inventory.Push(it->second);
                compost = it->second->comp();
                cyclus::CompMap cmap = compost->mass();
                cyclus::CompMap::iterator cit;
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
  //std::cout << "RX TRADE END" << std::endl;
}

/** Discharging fuel from the reactor*/
void ReactorX::GetMatlTrades(const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,cyclus::Material::Ptr> >& responses) {
    using cyclus::Material;
    using cyclus::Trade;
    //std::cout << "RX getTRADE START" << std::endl;
    std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
    //Remove the core loading
    if(shutdown == true){
        std::vector<cyclus::Material::Ptr> discharge = cyclus::ResCast<Material>(inventory.PopN(inventory.count()));
        fuel_library_.batch.clear();
        int i = 0;
        for (it = trades.begin(); it != trades.end(); ++it) {
            responses.push_back(std::make_pair(*it, discharge[i]));
            i++;
        }
    }else{
        //Remove the last batch from the core.
        cyclus::Material::Ptr discharge = cyclus::ResCast<Material>(inventory.Pop());
        fuel_library_.batch.erase(fuel_library_.batch.begin());
        for (it = trades.begin(); it != trades.end(); ++it) {
            responses.push_back(std::make_pair(*it, discharge));
        }
    }
    //std::cout << "RX getTRADE end" << std::endl;

}

/** This function converts a cyclus material point into a Bright-lite batch object. */
fuelBundle ReactorX::comp_function(cyclus::Material::Ptr mat1, fuelBundle &fuel_library_){
    //std::cout << "COMP FUNCTION" <<std::endl;
    cyclus::CompMap comp;
    cyclus::CompMap::iterator it;
    comp = mat1->comp()->mass(); //store the fractions of i'th batch in comp

    int comp_iso;
    fuelBundle temp_bundle = fuel_library_;
    batch_info info;
    temp_bundle.batch.clear();

    temp_bundle.batch.push_back(info);
    temp_bundle.batch[0].batch_fluence = 0;
    temp_bundle.batch[0].fraction.push_back(1);//fraction of the batch is set to one

    //each iso in comp
    for (it = comp.begin(); it != comp.end(); ++it){
        //std::cout<< it->first << ": " << it->second << std::endl;
        comp_iso = pyne::nucname::zzaaam(it->first);
        //each iso in all_iso
        for(int j = 0; j < temp_bundle.all_iso.size(); j++){
            int fl_iso = temp_bundle.all_iso[j].name;
            if(fl_iso == comp_iso && temp_bundle.batch[0].batch_fluence == 0){
                isoInformation temp_iso;
                temp_iso = temp_bundle.all_iso[j];
                temp_iso.fraction = it->second;
                //std::cout << "Name: " << it->first << "Amount " << it->second << std::endl;
                temp_bundle.batch[0].iso.push_back(temp_iso);
            }
        }
    }
    temp_bundle = StructReader(temp_bundle);
    if(CR_target > 0){
        temp_bundle = CoreCollapse(temp_bundle);
    } else {
        temp_bundle = fast_region_collapse(temp_bundle);
    }

    for(int i = 0; i < temp_bundle.batch.size(); i++){
        temp_bundle.batch[i].Fg = 0;
    }
    //std::cout << "COMP FUNCTION BYE" <<std::endl;
    return temp_bundle;
}

/** Builds a Bright-lite core from the batches existing on the reactor in the fuel
library.*/
void ReactorX::CoreBuilder(){
    //builds the necessary burnup calculation parameters
    // in fuel_library_

    //test to see if all fuel is fresh
    bool test = false;
    for(int i = 0; i < fuel_library_.batch.size(); i++){
        if(fuel_library_.batch[i].batch_fluence != 0){
            test = true;
        }
    }
    // if fuel is not fresh, do not reorder
    if(test == true){
        fuel_library_.batch[batches-1] = BatchCollapse(fuel_library_.batch[batches-1]);
        fuel_library_.batch[batches-1].Fg = 0;
        return;
    }
    if(struct_mode == 0){
        fuel_library_.struct_dest = 0;
        fuel_library_.struct_prod = 0;
        if(DA_mode != 0){
            std::cout << "Warning(Agent" << id() << "): DA wont contribute when struct mode (non-fuel contribution) is off." << std::endl;
        }
    } else {
        fuel_library_ = StructReader(fuel_library_);
    }
    fuel_library_ = CoreCollapse(fuel_library_);

    batch_reorder();

    return;
}

/** This functions orders the batches on the first tock. It is done to ensure that the
incoming batches are ordered by criticality to ensure that the batch with the lowest
k is discharged first */
void ReactorX::batch_reorder(){
//collapses each batch first, then orders them
//only needs to be called once per reactor start
//std::cout << " Begin batch_reorder" << std::endl;
    //begin ordering the batches
    fuelBundle temp_fuel = fuel_library_;
    fuel_library_.batch.clear();

    //goes through all the batches and orders them from lowest k to highest
    //uses temp_fuel to store the unordered batches
    double k0, k1;
    while(temp_fuel.batch.size() != 0){
        int lowest = 0;
        for(int i = 1; i < temp_fuel.batch.size(); i++){
            k0 = temp_fuel.batch[lowest].collapsed_iso.neutron_prod[1]/temp_fuel.batch[lowest].collapsed_iso.neutron_dest[1];
            k1 = temp_fuel.batch[i].collapsed_iso.neutron_prod[1]/temp_fuel.batch[i].collapsed_iso.neutron_dest[1];
            if(k1 < k0){
                //if k of i'th batch is smaller than k of current lowest batch
                lowest = i;
            }
        }
        fuel_library_.batch.push_back(temp_fuel.batch[lowest]);
        temp_fuel.batch.erase(temp_fuel.batch.begin() + lowest);
    }

//std::cout << " End batch_reorder" << std::endl;
}


extern "C" cyclus::Agent* ConstructReactorX(cyclus::Context* ctx) {
  return new ReactorX(ctx);
}

}
