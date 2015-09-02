#include "structures.h"

// Prints the isotope data to terminal
void IsoInfo::Print(int times) {
    std::cout << std::endl << "Isotope lib name: " << name << " fraction: "
              << fraction << std::endl;

    if (fluence.size() < 1) {
        std::cout << "  !Region not built." << std::endl;
        return;
    }

    if (times > fluence.size()) {times = fluence.size();}

    std::cout << "Fluence: ";
    for (int i = 0; i < times; i++) {
        std::cout << fluence[i] << " ";
    } std::cout << std::endl;

    std::cout << "NProd: ";
    for (int i = 0; i < times; i++) {
        std::cout << neutron_prod[i] << " ";
    } std::cout << std::endl;

    std::cout << "NDest: ";
    for (int i = 0; i < times; i++) {
        std::cout << neutron_dest[i] << " ";
    } std::cout << std::endl;

    std::cout << "BU: ";
    for (int i = 0; i < times; i++) {
        std::cout << BU[i] << " ";
    } std::cout << std::endl;

    std::cout << "----------------------------" << std::endl;

}


// Updates the iso in that region with the fraction
void RegionInfo::BuildIso(LibInfo library) {

    iso.name = 0; // Zero represents a collapsed iso and not a specific isotope

    // For every available isotope in the library
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

// Determines the burnup of the region based on the region fluence
float RegionInfo::CalcBU() {
    return CalcBU(fluence_);
}

// Determines the burnup of the region based on the given fluence
float RegionInfo::CalcBU(float fluence) {
    if(fluence <= 0){return 0;}
    if(fluence >= iso.fluence.back()){return iso.BU.back();}

    unsigned int ii;

    for(ii = 1; iso.fluence[ii] <= fluence; ii++){}

    return Interpolate(iso.BU[ii-1], iso.BU[ii], iso.fluence[ii-1], iso.fluence[ii], fluence);
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

// Prints the fluence of each region to terminal
void ReactorLiteInfo::PrintFluences() {
    std::cout << library_.name << " fluences: ";
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
        new_region.mass_ = core_mass_/regions_;
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

        // If a region has fluence the library must already have been built
        if (region[i].fluence_ == 0) {
            // Goes through each iso in comp
            for (it = comp.begin(); it != comp.end(); ++it){
                comp_iso = pyne::nucname::zzaaam(it->first);

                // For each isotope in all_iso
                for (int j = 0; j < library_.all_iso.size(); j++) {
                    if (library_.all_iso[j].name == comp_iso) {
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
            region[reg_i].BuildIso(library_);
        }
    }
}

// Reorders regions so that lowest k is at entry zero
void ReactorLiteInfo::Reorder() {
    std::vector<RegionInfo> temp_region = region;
    region.clear();
    float k0, k1;

    while(temp_region.size() != 0) {
        unsigned int lowest = 0;
        for(unsigned int remain_i = 1; remain_i < temp_region.size(); remain_i++) {
            // The first two discrete points are used as opposed to the very first
            k0 = (region[lowest].iso.neutron_prod[0] + region[lowest].iso.neutron_prod[1])
                 + (region[lowest].iso.neutron_dest[0] + region[lowest].iso.neutron_dest[1]);
            k1 = (region[remain_i].iso.neutron_prod[0] + region[remain_i].iso.neutron_prod[1])
                 + (region[remain_i].iso.neutron_dest[0] + region[remain_i].iso.neutron_dest[1]);

            if(k0 > k1) {
                lowest = remain_i;
            }
        }
        region.push_back(temp_region[lowest]);
        temp_region.erase(temp_region.begin() + lowest);
    }
}

// Returns the total burnup of the core by going through each batch
float ReactorLiteInfo::CalcBU() {
    float total_BU = 0;
    for(unsigned int reg_i = 0; reg_i < region.size(); reg_i++) {
        total_BU += region[reg_i].CalcBU();
    }
    return total_BU;
}


















