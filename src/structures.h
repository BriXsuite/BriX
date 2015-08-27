#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED

#include <utility>

#include "cyclus.h"

#include <string>
#include <cmath>
#include <sstream>


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
    float mod_Sig_a;     // Moderator Sig a
    float mod_Sig_s;     // Mod Sig s
    float fuel_Sig_s;    // Fuel Sig s
};

// Spatial flux calculation parameters for a given region
struct SpatialParamsX {
    float region_area;  // Total area of the region in [cm2]
    float Sig_a;        // Macroscopic absorption  cross section [cm-1]
    float Sig_tr;       // Macroscopic transport cross section [cm-1]
    float nuSig_f;      // Macroscopic fission cross section times nu [cm-1]
};

// Spatial flux calculation parameters for a given region
struct SpatialParamsLite {
    float delta;                // [cm]
    float fuel_area;            // Total area of the region in [cm2]
    float spatial_mod_thickness;// [cm]

    // All cross-sections in [cm]
    float spatial_mod_Sig_a;
    float spatial_mod_Sig_tr;
    float spatial_mod_Sig_f;
    float spatial_fuel_Sig_tr;
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
    float mass_;             // Mass of region, gets assigned at first tick

    IsoInfo iso;             // Collapsed, isoinfo for region
    std::map<int,float> fractions; // Name and fraction of each isotope

    unsigned int location_;  // Radial location of region, 1:center

    float fluence_ = 0;      // Fluence of this region
    float rflux_ = 1;            // Relative flux of region
    float DA = 1;

};

class ReactorLiteInfo {
public:
    // Initialized during startup
    unsigned int regions_;  // Total number of regions/batches
    float thermal_pow_;     // Reactor thermal power [MWth]
    float core_mass_;       // Total mass of all fuel in [kg]
    float target_BU_;       // Target burnup in [MWd/kgIHM]
    float target_CR_;       // Target conversion ratio
    float pnl;              // Nonleakage probability

    unsigned int flux_mode_;         // Flux calculation mode:
    // 0: Equal Power Share, 1:Uniform, 2:Inv.Neut.Prod, 3:Spatial

    SpatialParamsLite spatial_; // Spatial flux calculation parameters
    DisadvParams DA_;           // DA calculation parameters

    float struct_prod_ = 0;  // Non-fuel material neutron prod
    float struct_dest_ = 0;  // Non-fuel material neutron dest

    float fluence_timestep_; // Fluence propagation time step [day]

    LibInfo library_;

    // Regions are populated based on reactor parameters
    std::vector<RegionInfo> region;

    void UpdateFractions(std::vector<cyclus::Material::Ptr> manifest);
};

#endif // STRUCTURES_H_INCLUDED







