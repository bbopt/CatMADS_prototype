#ifndef __NOMAD_4_5_EVALUATORCONTROLPARAMETERS__
#define __NOMAD_4_5_EVALUATORCONTROLPARAMETERS__


#include "../Param/Parameters.hpp"
#include "../Param/RunParameters.hpp"

#include "../nomad_nsbegin.hpp"

/// The class for EvaluatorControl parameters that may be different between main threads.
/**
- Register all parameters during construction.
- Implement the checkAndComply function for sanity check.
*/
class DLL_UTIL_API EvaluatorControlParameters final : public Parameters
{
public:

    explicit EvaluatorControlParameters()
      : Parameters()
    {
        init();
    }

    /**
     The copy constructor is not implemented in the parent class to allow some control over what parameters can be copied or not. Use the deep copy function of parameters: Parameters::copyParameters.
     */
    EvaluatorControlParameters& operator=(const EvaluatorControlParameters& params) { copyParameters(params) ; return *this; }

    /**
     The copy constructor is not implemented in the parent class to allow some control over what parameters can be copied or not. Use the deep copy function of parameters: Parameters::copyParameters.
     */
    EvaluatorControlParameters(const EvaluatorControlParameters& params) : EvaluatorControlParameters() { copyParameters(params); }

    /// Check the sanity of parameters.
    /**
      By default, RunParameters is null. This method can be called in EvaluatorControl or anywhere during the optimization process.
      If a RunParameters is provided, more checks may be done.
     */
    void checkAndComply(const std::shared_ptr<NOMAD::EvaluatorControlGlobalParameters>& evaluatorControlGlobalParams = nullptr,
                        const std::shared_ptr<RunParameters>& runParams = nullptr);

private:

    /// Helper for constructor
    /**
     Register and set default values for all evaluator control attributes. The information to register all the attributes is contained in evaluatorControlAttributesDefinition.hpp as a set of strings to be interpreted. This file is created by the writeAttributeDefinition executable, called automatically by makefile when the evaluatorControlAttributeDefinition.txt file is modified.
     */
    void init() override;

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_EVALUATORCONTROLPARAMETERS__

