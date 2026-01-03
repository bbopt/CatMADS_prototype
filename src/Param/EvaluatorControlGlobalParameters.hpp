#ifndef __NOMAD_4_5_EVALUATORCONTROLGLOBALPARAMETERS__
#define __NOMAD_4_5_EVALUATORCONTROLGLOBALPARAMETERS__

#include "../Param/Parameters.hpp"
#include "../Param/PbParameters.hpp"

#include "../nomad_nsbegin.hpp"

/// The class for global EvaluatorControl parameters.
/**
- Register all parameters during construction.
- Implement the checkAndComply function for sanity check.
*/
class DLL_UTIL_API EvaluatorControlGlobalParameters final : public Parameters
{
public:

    explicit EvaluatorControlGlobalParameters()
      : Parameters()
    {
        init();
    }

    /// Check the sanity of parameters.
    void checkAndComply(const std::shared_ptr<PbParameters>& pbParams = nullptr);


private:

    /// Helper for constructor
    /**
     Register and set default values for all evaluator control attributes. The information to register all the attributes is contained in evaluatorControlAttributesDefinition.hpp as a set of strings to be interpreted. This file is created by the writeAttributeDefinition executable, called automatically by makefile when the evaluatorControlAttributeDefinition.txt file is modified.
     */
    void init() override;

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_EVALUATORCONTROLGLOBALPARAMETERS__

