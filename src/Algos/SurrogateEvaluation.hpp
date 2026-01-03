#ifndef __NOMAD_4_5_SURROGATE_EVALUATION__
#define __NOMAD_4_5_SURROGATE_EVALUATION__

#include "../Algos/IterationUtils.hpp"
#include "../Algos/QuadModel/QuadModelIteration.hpp"
#include "../Algos/Step.hpp"

#include "../nomad_nsbegin.hpp"

/// Class to evaluate trial points using static SURROGATE or MODEL
class SurrogateEvaluation : public Step
{
private:
    EvalType _evalType;
    std::unique_ptr<QuadModelIteration> _quadModelIteration;
    
    bool _evaluatorIsReady ;
    
    EvalPointSet & _trialPoints;
    
public:
    /// Constructor
    explicit SurrogateEvaluation(const Step* parentStep,
                                 EvalPointSet & trialPoints,
                                 EvalType evalType = EvalType::SURROGATE)
      : Step(parentStep),
       _trialPoints(trialPoints),
       _evalType(evalType),
       _quadModelIteration(nullptr),
       _evaluatorIsReady(false)
    {
        init();
    }

private:
    void init();

    virtual void startImp() override;   ///< Construct Model if evalType==MODEL
    virtual bool runImp() override;     ///< Evaluate points using static surrogate
    virtual void endImp() override;     ///<  Do nothing

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SURROGATE_EVALUATION__
