#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED

#include <utility>

#include "cyclus.h"

struct Daughter {
    unsigned int name;
    std::vector<float> mass;
    float fraction;
};

struct IsoInfo {
    unsigned int name;          // Nucid
    float fraction;    // Fraction of this isotope in the inherited class
    std::vector<float> neutron_prod;
    std::vector<float> neutron_dest;
    std::vector<float> BU; //total BU
    std::vector<float> fluence;
    std::vector<Daughter> iso_vector;
};

struct LibInfo {
    std::string name;               // Matches library in folder

    // Parameters used in library database generation if available
    float lib_flux;        // Flux in [n/cm2]
    float lib_res_t;       // Fuel residence time [day]
    float lib_power;       // Power in [MW(th)]
    float lib_mass;        // Mass in [kg]
    float lib_BU;          // Burnup in [MWd/kgIHM]
    float lib_CR;          // Conversion ratio

    std::vector<IsoInfo> all_iso;   // All iso's in the library go here
    float fraction;        // Fraction of this library in core region
};

// Information about the fuel region
class RegionInfo {
public:
    RegionInfo();

    float mass_;             // Mass of region
    float area_;             // Area of region [cm2]

    float nonf_prod_;        // Non-fuel material neutron prod
    float nonf_dest_;        // Non-fuel material neutron dest

    IsoInfo iso;             // Collapsed, isoinfo for region

    unsigned int location_;  // Radial location of region, 1:center

    float fluence_ = 0;      // Fluence of this region
    float rflux_;            // Relative flux of region


};

class ReactorLiteInfo {
public:
    ReactorLiteInfo();

    // Initialized during startup
    unsigned int regions_;  // Total number of regions/batches
    float power_;           // Reactor thermal power [MWth]
    float core_mass_;       // Total mass of all fuel in [kg]
    float target_BU_ = 0;   // Target burnup in [MWd/kgIHM]
    float target_CR_ = 0;   // Target conversion ratio
    float pnl;              // Nonleakage probability
    float CR_upper;         // Max mass number for CR fission product calc
    float CR_lower;         // Min mass number for CR calc

    // Regions are populated based on reactor parameters
    std::vector<RegionInfo> region;



};

#endif // STRUCTURES_H_INCLUDED







