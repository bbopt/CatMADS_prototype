#ifndef __NOMAD_4_5_RUNPARAMETERS__
#define __NOMAD_4_5_RUNPARAMETERS__

#include "../Param/EvaluatorControlGlobalParameters.hpp"
#include "../Param/Parameters.hpp"
#include "../Param/PbParameters.hpp"

#include "../nomad_nsbegin.hpp"

/// The class for the parameters defining the type of optimization/task to perform.
/**
The RunParameter are used by other parameters to update their value during sanity check.

- Register all parameters during construction.
- Implement the checkAndComply function for sanity check.
*/
class DLL_UTIL_API RunParameters final : public Parameters
{
private:
    static bool _warningUnknownParamShown;
    
    // Map direction type and variable group for poll
    // ChT. Probably not needed once a CatMads algorithm is available.
    std::map<NOMAD::DirectionType,NOMAD::ListOfVariableGroup> _mapDirTypeToVG;
    NOMAD::ListOfVariableGroup _fixVGForQMS;

public:
    // Constructor: init() will be called.
    // This will register and set default values to all attributes.
    explicit RunParameters()
    : Parameters()
    {
        init();
    }

    /// The copy constructor is not implemented in the parent class
    RunParameters& operator=(const RunParameters& params) { copyParameters(params) ; _mapDirTypeToVG =  params.getMapDirTypeToVG(); return *this; }
    RunParameters(const RunParameters& params) : RunParameters() { copyParameters(params); _mapDirTypeToVG =  params.getMapDirTypeToVG(); }

    /// Check the sanity of parameters.
    /**
     Register and set default values for all run attributes. The information to register all the attributes is contained in runAttributesDefinition.hpp as a set of strings to be interpreted. This file is created by the writeAttributeDefinition executable, called automatically by makefile when the runAttributeDefinition.txt file is modified.
    */
    void checkAndComply(const std::shared_ptr<EvaluatorControlGlobalParameters>& evaluatorControlGlobalParams,
                        const std::shared_ptr<PbParameters>& pbParams);
    
    
    // ChT. These set and get are temp methods for CatMads
    // Probably not needed once a CatMads algorithm is available.
    bool setMapDirTypeToVG(const std::shared_ptr<NOMAD::PbParameters>& pbParams, std::map<NOMAD::DirectionType,NOMAD::ListOfVariableGroup> &mapDirTypeToVG);
    const std::map<NOMAD::DirectionType,NOMAD::ListOfVariableGroup> & getMapDirTypeToVG() const  {return _mapDirTypeToVG;}
    bool setListFixVGForQuadModelSearch(const std::shared_ptr<NOMAD::PbParameters>& pbParams, const NOMAD::ListOfVariableGroup & _fixVGForQMS);
    const NOMAD::ListOfVariableGroup &  getListFixVGForQuadModelSearch() const {return _fixVGForQMS;}

private:
    /// Initialization
    /**
     * This will register and set default values to all attributes.
     */
    void init() override ;

    /// Helper for checkAndComply()
    void setStaticParameters();

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_RUNPARAMETERS__
