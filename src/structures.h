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
    float fraction;             // Fraction of this isotope in the inherited class
    std::vector<float> neutron_prod;
    std::vector<float> neutron_dest;
    std::vector<float> BU;      //total BU
    std::vector<float> fluence;
    std::vector<float> fission_products; // Fission product lump mass
    std::vector<Daughter> iso_vector;
};

struct nonActinide {
    int name;
    float sng;
    float scattering;
    float sn2n;
    float snp;
    float sngx;
    float sn2nx;
    float yyn;
    float total_prod;
    float total_dest;
};

// Thermal disadvantage factor parameters for a given region
struct DisadvParams {
    float a;            // Fuel radius [cm]
    float b;            // Moderator radius [cm]
    float mod_Siga;     // Moderator Sig a
    float mod_Sigs;     // Mod Sig s
    float fuel_Sigs;    // Fuel Sig s
};

// Spatial flux calculation parameters for a given region
struct SpatialParams {
    float region_area;  // Total area of the region in [cm2]
    float Sig_a;        // Macroscopic absorption  cross section [cm-1]
    float Sig_tr;       // Macroscopic transport cross section [cm-1]
    float nuSig_f;      // Macroscopic fission cross section times nu [cm-1]
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
    float mass_;             // Mass of region
    float area_;             // Area of region [cm2]

    float struct_prod_;      // Non-fuel material neutron prod
    float struct_dest_;      // Non-fuel material neutron dest

    IsoInfo iso;             // Collapsed, isoinfo for region
    SpatialParams spatial_;  // Spatial flux calculation info
    DisadvParams disadv_;    // DA calculation parameters

    unsigned int location_;  // Radial location of region, 1:center

    float fluence_ = 0;      // Fluence of this region
    float rflux_;            // Relative flux of region

};

class ReactorLiteInfo {
public:
    // Initialized during startup
    unsigned int regions_;  // Total number of regions/batches
    float power_;           // Reactor thermal power [MWth]
    float core_mass_;       // Total mass of all fuel in [kg]
    float target_BU_ = 0;   // Target burnup in [MWd/kgIHM]
    float target_CR_ = 0;   // Target conversion ratio
    float pnl;              // Nonleakage probability

    // Regions are populated based on reactor parameters
    std::vector<RegionInfo> region;



};

#endif // STRUCTURES_H_INCLUDED







