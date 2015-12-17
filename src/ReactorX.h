#ifndef CYCLUS_REACTORS_REACTORX_H_
#define CYCLUS_REACTORS_REACTORX_H_

#include <string>
#include <sstream>
#include <cmath>

#include "cyclus.h"
#include "librarytools.h"
#include "fuelfacilityx.h"

namespace reactorx {

/// @class ReactorX
///
/// The ReactorX class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
/// This agent is the full functionality BriX reactor.
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
///




class ReactorX : public cyclus::Facility  {
public:
    /// Constructor for ReactorX Class
    /// @param ctx the cyclus context for access to simulation-wide parameters
    explicit ReactorX(cyclus::Context* ctx);

    /// The Prime Directive
    /// Generates code that handles all input file reading and restart operations
    /// (e.g., reading from the database, instantiating a new object, etc.).
    /// @warning The Prime Directive must have a space before it! (A fix will be
    /// in 2.0 ^TM)

    #pragma cyclus

    /// A verbose printer for the Reactor-liteFacility
    virtual std::string str();

    /// The handleTick function specific to the ReactorX.
    /// @param time the time of the tick
    virtual void Tick();

    /// The handleTick function specific to the ReactorX.
    /// @param time the time of the tock
    virtual void Tock();

    std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();
    void AcceptMatlTrades(const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                        cyclus::Material::Ptr> >& responses);

    virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> GetMatlBids(
                          cyclus::CommodMap<cyclus::Material>::type& commod_requests);

    void GetMatlTrades( const std::vector< cyclus::Trade<cyclus::Material> >& trades,
            std::vector<std::pair<cyclus::Trade<cyclus::Material>,cyclus::Material::Ptr> >& responses);

    /**** General Reactor Parameters ****/

    #pragma cyclus var {"capacity": "max_inv_size"}
    cyclus::toolkit::ResourceBuff inventory;


        /** Required inputs **/
    #pragma cyclus var {"tooltip": "input commodity", \
                      "doc": "commodity that ReactorX consumes", \
                      "schematype": "token", \
                      "uitype": ["oneOrMore", "incommodity"]}
    std::vector<std::string> in_commods;

    #pragma cyclus var {"tooltip": "output commodity", \
                      "doc": "commodity that ReactorX supplies", \
                      "uitype": "outcommodity", \
                      "uilabel": "Output"}
    std::string out_commod;


        /** Inputs with defaults **/
    #pragma cyclus var {"default": 480, \
                      "units": "months", \
                      "userlevel": 1, \
                      "tooltip": "Time before reactor is shutdown after startup."}
    int reactor_life;

    #pragma cyclus var {"units": "MWth", \
                      "userlevel": 0, \
                      "tooltip": "Total thermal heat production of the core."}
    float thermal_pow;

    #pragma cyclus var {"default": 1e299, \
                      "userlevel": 2, \
                      "tooltip": "reactor maximum inventory size", \
                      "doc": "total maximum inventory size of the reactor"}
    float max_inv_size;

    #pragma cyclus var {"default": 0.33, \
                      "userlevel": 2, \
                      "tooltip": "Thermal to electric conversion rate."}
    float thermal_efficiency;

    #pragma cyclus var {"default": 10, \
                      "userlevel": 2, \
                      "tooltip": "Timestep [days] for the burnup calculation fluence progression."}
    float fluence_timestep;

    #pragma cyclus var {"default": 0.001, \
                      "userlevel": 3, \
                      "tooltip": "The absolute flux calculation convergence requirement."}
    float abs_flux_tol;

    #pragma cyclus var {"default": 0.01, \
                      "userlevel": 3, \
                      "tooltip": "The steady state fluence calculation convergence requirement."}
    float SS_tol;

    #pragma cyclus var {"default": 72, \
                        "userlevel": 2, \
                        "tooltip": "The amount of time fuel will spend in onsite storage", \
                        "doc": "This is the amount of time that the fuel will spend in onsite storage. This time period is used to allow short lived "  \
                                "fission products and actinides to decay away and make the fuel less radioactive. This effects the fuel composition as " \
                                "well as its current thermal power"}
    int decay_time_;

    /** Override Parameters **/
    #pragma cyclus var {"userlevel": 3, \
                      "default": 100, \
                      "tooltip": "The maximum number of cycles. Reactor is shutdown at this number."}
    int max_cycles;

    #pragma cyclus var {"userlevel": 3, \
                      "default": 0, \
                      "tooltip": "If a positive number is entered the calculated cycle length will be replaced by this value."}
    float cycle_length;


    /** Conversion Ratio (CR) Parameters **/
    #pragma cyclus var {"tooltip": "Target conversion ratio, set to negative for forward mode", \
                      "default": -1, \
                      "userlevel": 1, \
                      "units": "MWd/kgIHM"}
    float target_CR;

    #pragma cyclus var {"units": ["NUCID", "NUCID"], \
                      "tooltip": "List of fissile isotopes for conversion ratio calculation."}
    std::vector<std::string> CR_fissile;


private:
    /*
    ReactorXInfo reactor_core_;      // Class that holds core info on reactor
    cyclus::toolkit::ResourceBuff storage_;
    std::vector<int> decay_times_;
    bool shutdown_ = false;              // State of the reactor
    unsigned int cycle_end_;            // Reactor cycle end time
    unsigned int start_time_;           // Reactor start time
    unsigned int outage_remaining_ = 0; // Remaining outage time [in sim timestep] for refueling
    unsigned int refuels_ = 0;          // Number of times the reactor has been refueled
    float pow_frac_ = 0;
    float month_remainter_ = 0;
    float pow_per_time_ = 0;
    float pow_over_ = 0;
    float outage_time_ = 0;
*/

};

}  // namespace reactor

#endif  // CYCLUS_REACTORS_ReactorX_H_
