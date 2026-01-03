#ifndef __NOMAD_4_5_SURROGATE_EVALUATOR__
#define __NOMAD_4_5_SURROGATE_EVALUATOR__

#include "../Eval/Evaluator.hpp"

#include "../nomad_nsbegin.hpp"

/// Class for evaluating trial points as EvalType::SURROGATE.
class SurrogateEvaluator : public Evaluator
{
public:
    /// Constructor
    explicit SurrogateEvaluator(const std::shared_ptr<EvalParameters>& evalParams)
      : Evaluator(evalParams, EvalType::SURROGATE)
    {
    }

    virtual ~SurrogateEvaluator() {};

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SURROGATE_EVALUATOR__
