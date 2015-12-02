#ifndef CYCLUS_REACTORS_FUELFACILITYX_H_
#define CYCLUS_REACTORS_FUELFACILITYX_H_

#include <string>
#include <sstream>
#include <cmath>

#include "cyclus.h"
#include "librarytools.h"

namespace fuelfacilityx {

/// @class FuelFacilityX
///
/// The FuelFacilityX class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
/// This agent is the fuel type supplier facility for the full functionality BriX reactor.
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




class FuelFacilityX : public cyclus::Facility  {
public:
    /// Constructor for FuelFacilityX Class
    /// @param ctx the cyclus context for access to simulation-wide parameters
    explicit FuelFacilityX(cyclus::Context* ctx);

    /// The Prime Directive
    /// Generates code that handles all input file reading and restart operations
    /// (e.g., reading from the database, instantiating a new object, etc.).
    /// @warning The Prime Directive must have a space before it! (A fix will be
    /// in 2.0 ^TM)

    #pragma cyclus

    /// A verbose printer for the FuelFacilityX
    virtual std::string str();

    /// The handleTick function specific to the FuelFacilityX.
    /// @param time the time of the tick
    virtual void Tick();

    /// The handleTick function specific to the FuelFacilityX.
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
                      "doc": "commodity that reactor-lite consumes", \
                      "schematype": "token", \
                      "uitype": ["oneOrMore", "incommodity"]}
    std::vector<std::string> in_commods;

    #pragma cyclus var {"tooltip": "output commodity", \
                      "doc": "commodity that reactor-lite supplies", \
                      "uitype": "outcommodity", \
                      "uilabel": "Output"}
    std::string out_commod;

    #pragma cyclus var {"tooltip": "reactor libraries to load", \
                      "doc": "the reactor-lite's burnup & criticality library to use"}
    std::vector<std::string> libraries;

    #pragma cyclus var {"units": "MWth", \
                      "userlevel": 0, \
                      "tooltip": "Thermal heat production."}
    float thermal_pow;

    #pragma cyclus var {"units": "kgIHM", \
                      "userlevel": 0, \
                      "tooltip": "Total mass of the core."}
    float core_mass;

    #pragma cyclus var {"default": 1e299, \
                      "userlevel": 2, \
                      "tooltip": "reactor maximum inventory size", \
                      "doc": "total maximum inventory size of the reactor"}
    float max_inv_size;


private:
    /*
    FuelFacilityX reactor_core_;      // Class that holds core info on reactor
    cyclus::toolkit::ResourceBuff storage_;
*/

};

}  // namespace reactor

#endif  // CYCLUS_REACTORS_FUELFACILITYX_H_
