#ifndef CYCLUS_REACTORS_REACTORLITE_H_
#define CYCLUS_REACTORS_REACTORLITE_H_

#include <string>
#include <sstream>
#include <cmath>

#include "cyclus.h"
#include "librarytools.h"

namespace reactorlite {

/// @class ReactorLite
///
/// The ReactorLite class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
/// This agent is the lightweight BriX reactor.
///
/// @section agentparams Agent Parameters
/// Place a description of the required input parameters which define the
/// agent implementation.
///
/// @section optionalparams Optional Parameters
/// Place a description of the optional input parameters to define the
/// agent implementation.
///
/// @section detailed Detailed Behavior
/// The facility starts by reading the available isotope library databases.
/// A database is specific to the reactor and fuel type, therefore one reactor
/// may have more than one database describing it in cases where specific database
/// is not available, requiring an interpolation
/// to calculate a single isotope library database.
///
/// During the first tick the reactor performs the database interpolation (if
/// necessary) and builds the base parameters from the input file to fuel_library_.
/// All isotopes available for input fuel is placed in fuel_library_.all_iso
/// with correct names and isoinformation. This is independent of batches.
/// At the end of the first tick fuel_library_ has correct number of batches in
/// fuel_library_.batch[] all with zero fluence and no other information.
///
/// During the first material exchange the reactor receives fuel. The composition
/// of fuel is stored in resourcebuff inventory. Fuel may be blended, or a
/// composition may be forced to the reactor. The upstream facilities and
/// target burnup determines how the reactor will receive fuel. If more than one
/// in_commods is defined, then in_commods[0] will be used for refueling and
/// the others will be used for startup. If there are less in_commods than
/// there are batches to define the startup, in_commods[0] will used as a default.
///
/// At the beginning of first tock the reactor inventory is used to update the
/// composition of each batch in fuel_library_.iso[].fraction. Next, batch_reorder
/// is called if reactor is in startup. In this case the batcg libraries are built
/// for all batches the the core ordered.




class ReactorLite : public cyclus::Facility  {
public:
    /// Constructor for ReactorLite Class
    /// @param ctx the cyclus context for access to simulation-wide parameters
    explicit ReactorLite(cyclus::Context* ctx);

    /// The Prime Directive
    /// Generates code that handles all input file reading and restart operations
    /// (e.g., reading from the database, instantiating a new object, etc.).
    /// @warning The Prime Directive must have a space before it! (A fix will be
    /// in 2.0 ^TM)

    #pragma cyclus

    /// A verbose printer for the Reactor-liteFacility
    virtual std::string str();

    /// The handleTick function specific to the ReactorLite.
    /// @param time the time of the tick
    virtual void Tick();

    /// The handleTick function specific to the ReactorLite.
    /// @param time the time of the tock
    virtual void Tock();

    std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();
    void AcceptMatlTrades(const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                        cyclus::Material::Ptr> >& responses);

    /** General Reactor Parameters **/
    #pragma cyclus var {"tooltip": "input commodity", \
                      "doc": "commodity that the brightlite reactor consumes", \
                      "schematype": "token", \
                      "uitype": ["oneOrMore", "incommodity"]}
    std::vector<std::string> in_commods;

    #pragma cyclus var {"tooltip": "output commodity", \
                      "doc": "commodity that the brightlite supplies", \
                      "uitype": "outcommodity", \
                      "uilabel": "Output"}
    std::string out_commod;

    #pragma cyclus var {"tooltip": "reactor libraries to load", \
                      "doc": "the reactor's burnup & criticality behavior to use"}
    std::vector<std::string> libraries;

    #pragma cyclus var {"tooltip": ["interpolation pairs used for the library", \
                      "Interpolation metric", "Interpolation values"], \
                      "default": {}, \
                      "uitype": ["oneOrMore", "string", "double"]}
    std::map<std::string, double> interpol_pairs;

    #pragma cyclus var {"tooltip": "Number of regions/batches", \
                      "userlevel": 1, \
                      "default": 3}
    int regions;

    #pragma cyclus var {"tooltip": "Non-leakage probability of the core", \
                      "doc": "Varies from 0 - 1.", \
                      "userlevel": 1, \
                      "default": 0.98}
    float nonleakage;

    #pragma cyclus var {"default": 1e299, \
                      "userlevel": 2, \
                      "tooltip": "reactor maximum inventory size", \
                      "doc": "total maximum inventory size of the reactor"}
    float max_inv_size;

    #pragma cyclus var {"capacity": "max_inv_size"}
    cyclus::toolkit::ResourceBuff inventory;

    #pragma cyclus var {"tooltip": "Target burnup, set to zero for forward mode", \
                      "default": 0, \
                      "userlevel": 0, \
                      "units": "MWd/kgIHM"}
    float target_burnup;

    #pragma cyclus var {"tooltip": "Target conversion ratio, set to zero for forward mode", \
                      "default": 0, \
                      "userlevel": 0, \
                      "units": "MWd/kgIHM"}
    float target_CR;

    #pragma cyclus var {"units": "MWth", \
                      "userlevel": 0, \
                      "tooltip": "Thermal heat production."}
    float thermal_pow;

    #pragma cyclus var {"units": "kgIHM", \
                      "userlevel": 0, \
                      "tooltip": "Total mass of the core."}
    float core_mass;

    #pragma cyclus var {"default": 0.33, \
                      "userlevel": 2, \
                      "tooltip": "Thermal to electric conversion rate."}
    float thermal_efficiency;

    #pragma cyclus var {"default": 10, \
                      "userlevel": 2, \
                      "tooltip": "Timestep [days] for the burnup calculation fluence progression."}
    float fluence_timestep;

    #pragma cyclus var {"default": 0, \
                      "userlevel": 2, \
                      "tooltip": "Flux calculation method. 0: Equal Power Share, 1:Uniform, 2:Inv.Neut.Prod, 3:Spatial"}
    int flux_mode;



    /** Spatial Method Parameters **/
    #pragma cyclus var {"default": 5, \
                      "userlevel": 3, \
                      "tooltip": "Delta to be used for cylindrical flux calculation."}
    float spatial_delta;

    #pragma cyclus var {"default": 4197, \
                      "units": "cm2", \
                      "userlevel": 3, \
                      "tooltip": "Total area of the fuel (not including moderator) in core. Used for spatial flux calculation."}
    float spatial_area;

    #pragma cyclus var {"default": 50, \
                      "units": "cm", \
                      "userlevel": 3, \
                      "tooltip": "Radial thickness of the moderator used for cylindrical flux calculation. Used for spatial flux calculation."}
    float spatial_mod_thickness;

    #pragma cyclus var {"default": 0.0, \
                      "units": "cm-1", \
                      "userlevel": 3, \
                      "tooltip": "Macroscopic fission cross section of the moderator. Used for spatial flux calculation."}
    float spatial_mod_Sig_f;

    #pragma cyclus var {"default": 3.46, \
                      "units": "cm-1", \
                      "userlevel": 3, \
                      "tooltip": "Macroscopic transport cross section of the moderator. Used for spatial flux calculation."}
    float spatial_mod_Sig_tr;

    #pragma cyclus var {"default": 0.0222, \
                      "units": "cm-1", \
                      "userlevel": 3, \
                      "tooltip": "Macroscopic absorption cross section of the moderator. Used for spatial flux calculation."}
    float spatial_mod_Sig_a;

    #pragma cyclus var {"default": 0.12, \
                      "units": "cm-1", \
                      "userlevel": 3, \
                      "tooltip": "Macroscopic transport cross section of the fuel. Used for spatial flux calculation."}
    float spatial_fuel_Sig_tr;


    /** Fuel Disadvantage Factor (DA) Calculation Parameters **/
    #pragma cyclus var {"default": 0, \
                  "userlevel": 2, \
                  "tooltip": "Disadvantage calculation. 0:Off, 1:On"}
    int DA_mode;

    #pragma cyclus var {"default": 0.4095, \
                 "units": "cm", \
                  "userlevel": 3, \
                 "tooltip": "Fuel pin radius [cm] used for thermal disadvantage calculation."}
    float DA_a;

    #pragma cyclus var {"default": 0.70749, \
                 "units": "cm", \
                  "userlevel": 3, \
                 "tooltip": "Moderator radius [cm] used for thermal disadvantage calculation."}
    float DA_b;

    #pragma cyclus var {"default": 0.222, \
                 "units": "cm-1", \
                  "userlevel": 3, \
                 "tooltip": "Moderator macroscopic abs cross-section for thermal disadvantage calculation."}
    float DA_mod_Sig_a;

    #pragma cyclus var {"default": 3.44, \
                 "units": "cm-1", \
                  "userlevel": 3, \
                 "tooltip": "Moderator macroscopic scattering cross-section for thermal disadvantage calculation."}
    float DA_mod_Sig_s;

    #pragma cyclus var {"default": 0.43, \
                 "units": "cm-1", \
                  "userlevel": 3, \
                 "tooltip": "Fuel macroscopic scattering cross-section for thermal disadvantage calculation."}
    float DA_fuel_Sig_s;


private:
    ReactorLiteInfo reactor_core_;      // Class that holds core info on reactor
    bool shutdown_ = false;              // State of the reactor
    unsigned int cycle_end_;            // Reactor cycle end time
    unsigned int start_time_;           // Reactor start time
    unsigned int outage_remaining_ = 0; // Remaining outage time [in sim timestep] for refueling
    float pow_frac_ = 0;
    float month_remainter_ = 0;

};

}  // namespace reactor

#endif  // CYCLUS_REACTORS_REACTORLITE_H_
