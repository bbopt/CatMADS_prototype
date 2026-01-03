#ifndef __NOMAD_4_5_EVALPARAMETERS__
#define __NOMAD_4_5_EVALPARAMETERS__

#include "../Param/EvaluatorControlGlobalParameters.hpp"
#include "../Param/EvaluatorControlParameters.hpp"
#include "../Param/Parameters.hpp"
#include "../Param/RunParameters.hpp"

#include "../nomad_nsbegin.hpp"

/// Class for Evaluator parameters
/**
- Register all parameters during construction.
- Implement the checkAndComply function for sanity check.
- The parameters control the behavior of the evaluator, the evaluation of outputs and the computation of h.
*/
class DLL_UTIL_API EvalParameters final : public Parameters
{
public:

    explicit EvalParameters()
    : Parameters()
    {
        init();
    }

    /// Check the sanity of parameters.
    void checkAndComply(const std::shared_ptr<RunParameters>& runParams,
                        const std::shared_ptr<PbParameters>& pbParams,
                        const std::shared_ptr<EvaluatorControlGlobalParameters>& evaluatorControlGlobalParams,
                        const std::shared_ptr<EvaluatorControlParameters>& evaluatorControlParams);
    
    /**
     The copy constructor is not implemented in the parent class to allow some control over what parameters can be copied or not. Use the deep copy function of parameters: Parameters::copyParameters.
     */
    EvalParameters& operator=(const EvalParameters& params) { copyParameters(params) ; return *this; }
    
    /**
     The copy constructor is not implemented in the parent class to allow some control over what parameters can be copied or not. Use the deep copy function of parameters: Parameters::copyParameters.
     */
    EvalParameters(const EvalParameters& params) : EvalParameters() { copyParameters(params); }

private:
    /// Helper for constructor
    /**
     Register and set default values for all evaluation attributes. The information to register all the attributes is contained in evalAttributesDefinition.hpp as a set of strings to be interpreted. This file is created by the writeAttributeDefinition executable, called automatically by makefile when the evalAttributeDefinition.txt file is modified.
     */
    void init() override ;
    

    /// Helper for checkAndComply()
    void updateExeParam(const std::shared_ptr<NOMAD::RunParameters>& runParams, const std::string& paramName);

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_EVALPARAMETERS__

