#include "structures.h"


std::map<std::string, LibInfo> global_libs;
// Prints the isotope data to terminal
void IsoInfo::Print(int times) {
    std::cout << std::endl << "Isotope lib name: " << name << " fraction: "
              << fraction << " k[0]: " << neutron_prod[0]/neutron_dest[0] << std::endl;

    if (fluence.size() < 1) {
        std::cout << "  !Region not built." << std::endl;
        return;
    }

    if (times > fluence.size()) {times = fluence.size();}

    std::cout << "  Fluence: ";
    for (int i = 0; i < times; i++) {
        std::cout << fluence[i] << " ";
    } std::cout << std::endl;

    std::cout << "  NProd: ";
    for (int i = 0; i < times; i++) {
        std::cout << neutron_prod[i] << " ";
    } std::cout << std::endl;

    std::cout << "  NDest: ";
    for (int i = 0; i < times; i++) {
        std::cout << neutron_dest[i] << " ";
    } std::cout << std::endl;

    std::cout << "  BU: ";
    for (int i = 0; i < times; i++) {
        std::cout << BU[i] << " ";
    } std::cout << std::endl;

    std::cout << "  ------------" << std::endl;

}

void RegionInfo::Print() {
    std::cout << "Fluence: " << fluence_ << " rFlux: " << rflux_ << " mass: " << mass_;
    iso.Print();
}

// Updates the iso in that region with the fraction
void RegionInfo::BuildIso(LibInfo library) {

    iso.name = 0; // Zero represents a collapsed iso and not a specific isotope
    //For every available isotope in the library
    for (unsigned int lib_i = 0; lib_i < library.all_iso.size(); lib_i++) {
        // Save the name of the current library isotope
        unsigned const int iso_name = library.all_iso[lib_i].name;
        //
        if (library.all_iso[lib_i].iso_vector.size() > 0) {
            if (iso.fluence.size() < 1) {
                for (int i = 0; i < library.all_iso[lib_i].fluence.size(); i++) {
                    iso.fluence.push_back(library.all_iso[lib_i].fluence[i]);
                }
                for (int i = 0; i < library.all_iso[lib_i].neutron_prod.size(); i++) {
                    iso.neutron_prod.push_back(fractions[iso_name] * library.all_iso[lib_i].neutron_prod[i]);
                }
                for (int i = 0; i < library.all_iso[lib_i].neutron_dest.size(); i++) {
                    iso.neutron_dest.push_back(fractions[iso_name] * library.all_iso[lib_i].neutron_dest[i]);
                }
                for (int i = 0; i < library.all_iso[lib_i].BU.size(); i++) {
                    iso.BU.push_back(fractions[iso_name] * library.all_iso[lib_i].BU[i]);
                }
                for (int i = 0; i < library.all_iso[lib_i].iso_vector.size(); i++) {
                    iso.iso_vector.push_back(library.all_iso[lib_i].iso_vector[i]);
                    for(int k = 0; k < iso.iso_vector[i].mass.size(); k++) {
                        iso.iso_vector[i].mass[k] = fractions[iso_name] * iso.iso_vector[i].mass[k];
                    }
                }
            } else {

                for (int i = 0; i < library.all_iso[lib_i].neutron_prod.size(); i++) {
                    iso.neutron_prod[i] += fractions[iso_name] * library.all_iso[lib_i].neutron_prod[i];
                }
                for (int i = 0; i < library.all_iso[lib_i].neutron_dest.size(); i++) {
                    iso.neutron_dest[i] += fractions[iso_name] * library.all_iso[lib_i].neutron_dest[i];
                }
                for (int i = 0; i < library.all_iso[lib_i].BU.size(); i++) {
                    iso.BU[i] += fractions[iso_name] * library.all_iso[lib_i].BU[i];
                }
                for (int i = 0; i < library.all_iso[lib_i].iso_vector.size(); i++) {
                    bool iso_check = true;
                    for (int j = 0; j < iso.iso_vector.size(); j++) {
                        if (library.all_iso[lib_i].iso_vector[i].name == iso.iso_vector[j].name) {
                            for (int k = 0; k < iso.iso_vector[j].mass.size(); k++) {
                                for (int ii = 0; ii < library.all_iso[lib_i].iso_vector[i].mass.size(); ii ++) {
                                    if ( k ==ii ) {
                                        iso.iso_vector[j].mass[k] += fractions[iso_name] *library.all_iso[lib_i].iso_vector[i].mass[ii];
                                    }
                                }
                            }
                            iso_check = false;
                        }
                    }
                    if (iso_check == true) {
                        iso.iso_vector.push_back(library.all_iso[lib_i].iso_vector[i]);
                        for(int k = 0; k < iso.iso_vector[iso.iso_vector.size()-1].mass.size()-1; k++) {
                            iso.iso_vector[iso.iso_vector.size()-1].mass[k] = iso.iso_vector[iso.iso_vector.size()-1].mass[k]*fractions[iso_name] ;
                        }
                    }
                }
            }
        } else {
            if (iso.neutron_prod.size() > 1) {
                for (int i = 0; i < library.all_iso[lib_i].neutron_prod.size(); i++) {
                    iso.neutron_prod[i] = iso.neutron_prod[i] + fractions[iso_name] *library.all_iso[lib_i].neutron_prod[i];
                }
                for (int i = 0; i < library.all_iso[lib_i].neutron_dest.size(); i++) {
                    iso.neutron_dest[i] = iso.neutron_dest[i] + fractions[iso_name] *library.all_iso[lib_i].neutron_dest[i];
                }
            } else {
                for (int i = 0; i < library.all_iso[lib_i].neutron_prod.size(); i++) {
                    iso.neutron_prod.push_back(fractions[iso_name] *library.all_iso[lib_i].neutron_prod[i]+(fractions[iso_name] )*library.all_iso[1].neutron_prod[i]);
                }
                for (int i = 0; i < library.all_iso[lib_i].neutron_dest.size(); i++) {
                    iso.neutron_dest.push_back(fractions[iso_name] *library.all_iso[lib_i].neutron_dest[i]+(fractions[iso_name] )*library.all_iso[1].neutron_dest[i]);
                }
            }
        }
    }
}

void IsoInfo::BuildCRVector(std::vector<int> fissile_isos) {
    float cr;
    double delta_fiss = 0;
    double delta_fp = 0;
    CR.push_back(0.);
    for(unsigned int f = 1; f < fluence.size(); f++){ 
        for(unsigned int nuc = 0; nuc < iso_vector.size(); nuc++) {
            unsigned int zzaaa = iso_vector[nuc].name;
            unsigned int zz = zzaaa % 100000;
            if(zz > 20 && zz < 80){
                delta_fp += iso_vector[nuc].mass[f] - iso_vector[nuc].mass[0];
            } else if (zz >= 80){
                for(unsigned int j = 0; j < fissile_isos.size(); j++){
                    if (fissile_isos[j] == zzaaa){
                        delta_fiss += iso_vector[nuc].mass[f] - iso_vector[nuc].mass[0];
                    }
                } 
            }   
        }
        if(delta_fp >= 0){
            CR.push_back((delta_fp+delta_fiss)/delta_fp);
        } else {
            CR.push_back(0.);
        }       
        delta_fp = 0.;
        delta_fiss = 0.;
    }
    return;    
}

// Determines the composition of the given isotope at region fluence
void RegionInfo::UpdateComp() {
    if(fluence_ <= 0) {return;}
    if(fluence_ >= iso.fluence.back()) {
        std::cout << " Warning! Region fluence exceeds library range in composition calculation!"
                << " Composition will not be updated." << std::endl;
        return;
    }

    unsigned int ii;

    for(ii = 1; iso.fluence[ii] <= fluence_; ii++){}

    // Finds the interpolation slope to use for faster interpolation
    const double slope = (fluence_ - iso.fluence[ii-1]) / (iso.fluence[ii] - iso.fluence[ii-1]);

    for(int iso_i = 0; iso_i < iso.iso_vector.size(); iso_i++) {
        comp[iso.iso_vector[iso_i].name] = (iso.iso_vector[iso_i].mass[ii-1] +
                (iso.iso_vector[iso_i].mass[ii] - iso.iso_vector[iso_i].mass[ii-1]) * slope) / 1000;
    }
}

// Determines the composition of a region at a specific fluence. 
std::map<int, float> RegionInfo::FindComp(float fluence) {
    std::map<int, float> temp_comp;
    if(fluence <= 0) {return temp_comp;}
    if(fluence >= iso.fluence.back()) {
        std::cout << " Warning! Region fluence exceeds library range in composition calculation!"
                << " Composition will not be updated." << std::endl;
        return temp_comp;
    }

    unsigned int ii;

    for(ii = 1; iso.fluence[ii] <= fluence; ii++){}

    // Finds the interpolation slope to use for faster interpolation
    const double slope = (fluence - iso.fluence[ii-1]) / (iso.fluence[ii] - iso.fluence[ii-1]);

    for(int iso_i = 0; iso_i < iso.iso_vector.size(); iso_i++) {
        temp_comp[iso.iso_vector[iso_i].name] = (iso.iso_vector[iso_i].mass[ii-1] +
                (iso.iso_vector[iso_i].mass[ii] - iso.iso_vector[iso_i].mass[ii-1]) * slope) / 1000;
    }
    return temp_comp;
}

// Determines the composition of a region at initial loading. 
std::map<int, float> RegionInfo::InitComp() {
    std::map<int, float> temp_comp;
    for(int iso_i = 0; iso_i < iso.iso_vector.size(); iso_i++) {
        temp_comp[iso.iso_vector[iso_i].name] = iso.iso_vector[iso_i].mass[0];
    }
    return temp_comp;
}

// Determines the difference between to init and current composition of the region.
std::map<int, float> RegionInfo::CompDiff(float fluence) {
    std::map<int, float> diff_comp;
    std::map<int, float> init_comp = InitComp();
    std::map<int, float> temp_comp = FindComp(fluence);
    for(unsigned int i = 0; i < iso.iso_vector.size(); i++){
        unsigned int zz = iso.iso_vector[i].name;
        diff_comp[zz] = temp_comp[zz] - init_comp[zz];             
    }
    return diff_comp;
}

// Determines the burnup/mass of the region based on the region fluence
float RegionInfo::CalcBU() {
    return CalcBU(fluence_);
}

// Determines the burnup/mass of the region based on the given fluence
float RegionInfo::CalcBU(float fluence) {
    if(fluence <= 0) {return 0;}
    if(fluence >= iso.fluence.back()) {return iso.BU.back();}
    unsigned int ii;
    float burnup;
    
    for(ii = 1; iso.fluence[ii] <= fluence; ii++){}

    burnup = Interpolate(iso.BU[ii-1], iso.BU[ii], iso.fluence[ii-1], iso.fluence[ii], fluence);
    if(burnup < 0) {return 0;}
    return burnup;
}

float RegionInfo::CalcCR() {
    return CalcCR(fluence_);
}

/** Finds the conversion ratio (based on user inputted isotopes) of the entire core **/
float RegionInfo::CalcCR(float fluence) {
    if(fluence <= 0) {return 0;}
    if(fluence >= iso.fluence.back()) {return iso.CR.back();}
    unsigned int ii;
    float cr;
    
    for(ii = 1; iso.fluence[ii] <= fluence; ii++){}

    cr = Interpolate(iso.BU[ii-1], iso.BU[ii], iso.fluence[ii-1], iso.fluence[ii], fluence);
    if(cr < 0) {return 0;} 
    return cr;
}


// Determines the neutron production of the region based on region fluence
float RegionInfo::CalcProd() {
    return CalcProd(fluence_);
}

// Determines the neutron production of the region based on given fluence
float RegionInfo::CalcProd(float fluence) {
    if(fluence < 0){return 0;}
    if(fluence >= iso.fluence.back()){return iso.neutron_prod.back();}

    unsigned int ii;

    for(ii = 1; iso.fluence[ii] <= fluence; ii++){}

    return Interpolate(iso.neutron_prod[ii-1], iso.neutron_prod[ii],
                       iso.fluence[ii-1], iso.fluence[ii], fluence);
}

// Determines the neutron production of the region based on region fluence
float RegionInfo::CalcDest() {
    return CalcDest(fluence_);
}

// Determines the neutron destruction of the region based on given fluence
float RegionInfo::CalcDest(float fluence) {
    if(fluence < 0){return 0;}
    if(fluence >= iso.fluence.back()){return iso.neutron_dest.back();}

    unsigned int ii;

    for(ii = 1; iso.fluence[ii] <= fluence; ii++){}

    return Interpolate(iso.neutron_dest[ii-1], iso.neutron_dest[ii],
                       iso.fluence[ii-1], iso.fluence[ii], fluence);
}

// Determines nu Sig_f [cm-1] from neutron production rate
float RegionInfo::CalcNuSigf() {
    return CalcProd() * 0.01097;
}

// Determines Sig_a [cm-1] from neutron destruction rate
float RegionInfo::CalcSiga() {
    return CalcDest() * 0.01097;
}

// Checks variables for consistency
bool ReactorLiteInfo::ConsistencyCheck() {
    unsigned int regions_;  // Total number of regions/batches
    float thermal_pow_;     // Reactor thermal power [MWth]
    float core_mass_;       // Total mass of all fuel in [kg]
    float target_BU_;       // Target burnup in [MWd/kgIHM]
    float target_CR_;       // Target conversion ratio
    float pnl;              // Nonleakage probability
    float fluence_timestep_;// Fluence propagation time step [second]
    float base_flux_;       // Library base flux or last cycle flux
    std::vector<int> CR_fissile_; // List of fissile isotopes for CR calc

    float SS_tol_;          // Convergence tolerance for steady state fluence calculation

    if(flux_mode_ < 0 || flux_mode_ > 3) {
        std::cout << "Unrecognized flux mode " << flux_mode_ << std:: endl;
        return false;
    }

    if(DA_mode_ != 0 || DA_mode_ != 1) {
        std::cout << "Unrecognized DA mode " << DA_mode_ << std::endl;
        return false;
    }

    if(abs_flux_tol_ < 0.00001 || SS_tol_ < 0.00001) {
        return false;
    }

    if(abs_flux_tol_ > 0.4 || SS_tol_ > 0.4) {
        return false;
    }


}

// Prints the iso of each region
void ReactorLiteInfo::PrintRegionIsos() {
    std::cout << global_libs[libraries_[0]].name << " region isos:" << std::endl;
    for(unsigned int reg_i = 0; reg_i < region.size(); reg_i++) {
        region[reg_i].Print();;
    }
    std::cout << "-------------------------------" << std::endl;
}

// Prints the fluence of each region to terminal
void ReactorLiteInfo::PrintFluences() {
    std::cout << global_libs[libraries_[0]].name << " fluences: ";
    for(unsigned int reg_i = 0; reg_i < region.size(); reg_i++) {
        std::cout << region[reg_i].fluence_ << "  ";
    }
    std::cout << std::endl;
}

// Updates the fractions of regions, builds new regions if they're empty
void ReactorLiteInfo::UpdateFractions(std::vector<cyclus::Material::Ptr> manifest) {
    unsigned const int manifest_size = manifest.size();

    if (manifest_size != regions_) {
        std::cout << "ReactorLite manifest size and region size mismatch!" << std::endl;
    }

    // Number of regions consistency check and fix
    if (manifest_size > region.size()) {
        RegionInfo new_region;
        for (int i = 0; i < manifest_size - region.size(); i++){
            region.push_back(new_region);
        }
    }

    cyclus::CompMap comp;
    cyclus::CompMap::iterator it;

    // Puts new isotope libraries into the reactorlite libraries for the new region
    for (int i = 0; i < manifest_size; i++) {
        // Builds correct isoinfo and fraction of every isotope in each batch
        comp = manifest[i]->comp()->mass(); //store the fractions of i'th batch in comp
        int comp_iso;

        // Update mass
        region[i].mass_ = manifest[i]->quantity();

        // If a region has fluence the library must already have been built
        if (region[i].fluence_ == 0) {
            // Goes through each iso in comp
            for (it = comp.begin(); it != comp.end(); ++it){
                comp_iso = pyne::nucname::zzaaam(it->first);

                // For each isotope in all_iso
                for (int j = 0; j < global_libs[libraries_[0]].all_iso.size(); j++) {
                    if (global_libs[libraries_[0]].all_iso[j].name == comp_iso) {
                        region[i].fractions[comp_iso] = it->second;
                    }
                }
            }
        } // nonzero fluence
    } // Manifest_size, i
}

// Goes through all regions and builds their isos
void  ReactorLiteInfo::BuildRegionIsos() {
    for (unsigned int reg_i = 0; reg_i < region.size(); reg_i++) {
        // If a region has zero fluence the iso must not be built yet
        if (region[reg_i].fluence_ == 0) {
            region[reg_i].BuildIso(global_libs[libraries_[0]]);
        }
    }
}

// Reorders regions so that lowest k is at entry zero
void ReactorLiteInfo::Reorder() {
    std::vector<RegionInfo> temp_region = region;
    region.clear();
    double k0, k1;

    while(temp_region.size() != 0) {
        unsigned int lowest = 0;
        for(unsigned int remain_i = 1; remain_i < temp_region.size(); remain_i++) {
            // The first two discrete points are used as opposed to the very first
            k0 = temp_region[lowest].iso.neutron_prod[0]
                 / temp_region[lowest].iso.neutron_dest[0];
            k1 = (temp_region[remain_i].iso.neutron_prod[0])
                 / (temp_region[remain_i].iso.neutron_dest[0]);

            if(k0 > k1) {
                lowest = remain_i;
            }
        }
        region.push_back(temp_region[lowest]);
        temp_region.erase(temp_region.begin() + lowest);
    }
}

// Updates the composition of isotopes in all regions
void ReactorLiteInfo::UpdateComp() {
    for(unsigned int reg_i = 0; reg_i < region.size(); reg_i++) {
        region[reg_i].UpdateComp();
    }
}

// Returns the total burnup/mass of the core by going through each batch
float ReactorLiteInfo::CalcBU() {
    unsigned const int regions = region.size();
    float total_BU = 0;
    for(unsigned int reg_i = 0; reg_i < regions; reg_i++) {
        total_BU += region[reg_i].CalcBU() * region[reg_i].mass_;
    }
    return total_BU / core_mass_;
}

// Returns the burnup if the reactor at given flux; using rflux_ and fluence_timestep_
float ReactorLiteInfo::CalcBU(float flux) {
    unsigned const int regions = region.size();
    float burnup = 0;
    float fluence;

    for(int reg_i = 0; reg_i < regions; reg_i++) {
        fluence = region[reg_i].fluence_ + (region[reg_i].rflux_ * flux * fluence_timestep_);
        burnup += region[reg_i].CalcBU(fluence) * region[reg_i].mass_;
    }

    return burnup / core_mass_;
}















