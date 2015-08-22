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
    double nonleakage;

    #pragma cyclus var {"default": 1e299, \
                      "userlevel": 2, \
                      "tooltip": "reactor maximum inventory size", \
                      "doc": "total maximum inventory size of the reactor"}
    double max_inv_size;

    #pragma cyclus var {"tooltip": "Target burnup, if reactor in forward mode", \
                      "default": 0, \
                      "userlevel": 0, \
                      "units": "MWd/kgIHM"}
    double target_burnup;

    #pragma cyclus var {"units": "MWe", \
                      "userlevel": 0, \
                      "tooltip": "Electrical production."}
    double generated_power;

    #pragma cyclus var {"units": "kgIHM", \
                      "userlevel": 0, \
                      "tooltip": "Total mass of the core."}
    double core_mass;

    #pragma cyclus var {"default": 0.33, \
                      "userlevel": 2, \
                      "tooltip": "Thermal to electric conversion rate."}
    double thermal_efficiency;


private:
    ReactorLiteInfo reactor_core_;
    bool shutdown = false;
    bla blavar;

};

}  // namespace reactor

#endif  // CYCLUS_REACTORS_REACTORLITE_H_
