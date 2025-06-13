/**
 \file   ComputeSuccessType.cpp
 \brief  Comparison methods for EvalPoints (implementation)
 \author Viviane Rochon Montplaisir
 \date   March 2017 / September 2020
 \see    ComputeSuccessType.hpp
 */
#include "../Eval/ComputeSuccessType.hpp"

/*--------------------------*/
/* Class ComputeSuccessType */
/*--------------------------*/


NOMAD::SuccessType NOMAD::ComputeSuccessType::operator()(const NOMAD::EvalPointCstPtr &p1,
                                                         const NOMAD::EvalPointCstPtr &p2,
                                                         const NOMAD::Double& hMax)
{
    auto compactComputeType = _computeType.Short();
    auto computeType = compactComputeType.computeType;
    auto evalType = _computeType.evalType;
    
    if (NOMAD::EvalType::UNDEFINED == evalType)
    {
        std::string err = "Cannot compute success for undefined eval type";
        throw NOMAD::Exception(__FILE__,__LINE__,err);
    }
    
    NOMAD::SuccessType success = NOMAD::SuccessType::UNDEFINED;
    
    if (nullptr != p1)
    {
        if (nullptr == p2)
        {
            if (NOMAD::ComputeType::PHASE_ONE == computeType)
            {
                success = NOMAD::SuccessType::FULL_SUCCESS;
            }
            else if (NOMAD::ComputeType::STANDARD == computeType || NOMAD::ComputeType::DMULTI_COMBINE_F == computeType || NOMAD::ComputeType::USER == computeType)
            {
                
                NOMAD::Double h = p1->getH(_computeType);
                if (!h.isDefined() || h > hMax || h == NOMAD::INF)
                {
                    // Even if evalPoint2 is NULL, this case is still
                    // not a success.
                    success = NOMAD::SuccessType::UNSUCCESSFUL;
                }
                else if (p1->isFeasible(_computeType))
                {
                    // New feasible point: full success
                    success = NOMAD::SuccessType::FULL_SUCCESS;
                }
                else
                {
                    // New infeasible makes for partial success, not full success
                    success = NOMAD::SuccessType::PARTIAL_SUCCESS;
                }
            }
            else
            {
                std::string err = "Compute success type function for " + NOMAD::computeTypeToString(computeType) + " not available";
                throw NOMAD::Exception(__FILE__,__LINE__,err);
            }
        }
        else
        {
            success = NOMAD::Eval::computeSuccessType(p1->getEval(evalType),
                                                      p2->getEval(evalType),
                                                      compactComputeType,
                                                      hMax);
        }
    }
    
    return success;
    
    
}


