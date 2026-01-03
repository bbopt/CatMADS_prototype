
#include "../../Algos/EvcInterface.hpp" // To access EvalType
#include "../../Algos/NelderMead/NMSimplexEvalPoint.hpp"
#include "../../Eval/ComputeSuccessType.hpp"


bool NOMAD::NMSimplexEvalPointCompare::operator()(const NOMAD::EvalPoint& lhs,
                                                  const NOMAD::EvalPoint& rhs) const
{
    // Workaround to get EvalType for ComputeSuccessType.
    NOMAD::EvalType evalType = NOMAD::EvalType::BB;
    NOMAD::FHComputeTypeS computeType /* default initializer*/;
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    if (nullptr != evc)
    {
        evalType = evc->getCurrentEvalType();
        computeType = evc->getFHComputeTypeS();
    }
    NOMAD::FHComputeType completeComputeType = {evalType, computeType};
    
    NOMAD::ComputeSuccessType computeSuccess(completeComputeType);

    // Create a "temporary" shared_ptr from an existing eval point. Do not copy. Call empty deleter at exit of this function.
    NOMAD::EvalPointCstPtr lhsPtr(&lhs, [](const NOMAD::EvalPoint* p){});
    NOMAD::EvalPointCstPtr rhsPtr(&rhs, [](const NOMAD::EvalPoint* p){});
    NOMAD::SuccessType success = computeSuccess(lhsPtr, rhsPtr);

    if (success >= NOMAD::SuccessType::FULL_SUCCESS)
    {
        return true;
    }

    success = computeSuccess(rhsPtr, lhsPtr);
    if (success >= NOMAD::SuccessType::FULL_SUCCESS)
    {
        return false;
    }
    

    // No dominance, compare h values.
    NOMAD::Double h1 = lhs.getH(completeComputeType);
    NOMAD::Double h2 = rhs.getH(completeComputeType);
    if (h1.isDefined() && h2.isDefined())
    {
        if (h1 < h2)
        {
            return true;
        }
        if (h2 < h1)
        {
            return false;
        }
    }
    else if (h1.isDefined() && !h2.isDefined())
    {
        return true;
    }
    else if (!h1.isDefined() && h2.isDefined())
    {
        return false;
    }

    // The "older" function from NM-Mads paper is implemented by comparing the tags
    return lhs.getTag() < rhs.getTag();
}

