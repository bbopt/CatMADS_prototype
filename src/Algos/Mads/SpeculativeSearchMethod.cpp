/**
 \file   SpeculativeSearchMethod.cpp
 \brief  Speculative search (implementation)
 \author Christophe Tribes and Sebastien Le Digabel
 \date   2018-03-1
 */
#include "../../Algos/Mads/SpeculativeSearchMethod.hpp"
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Output/OutputQueue.hpp"

/*-------------------------------------------------------------*/
/*                     MADS speculative search                 */
/*-------------------------------------------------------------*/
/* Multiple points: i=1, ..., SPECULATIVE_SEARCH_MAX           */
/* d: direction of last success scaled to intersect the frame  */
/*  x_t = x_{k-1} + d * i                                      */
/*-------------------------------------------------------------*/

void NOMAD::SpeculativeSearchMethod::init()
{
    setStepType(NOMAD::StepType::SEARCH_METHOD_SPECULATIVE);

    bool enabled = false;
    // For some testing, it is possible that _runParams is null
    if (nullptr != _runParams)
    {
        enabled = _runParams->getAttributeValue<bool>("SPECULATIVE_SEARCH");
    }
    setEnabled(enabled);
    
    // Number of speculative search trial points for a pass
    _nbSearches = 0;
    _baseFactor = 0.0;
    if (nullptr != _runParams)
    {
        _nbSearches = _runParams->getAttributeValue<size_t>("SPECULATIVE_SEARCH_MAX");
        
        // Base factor to control the extent of the speculative direction
        _baseFactor = _runParams->getAttributeValue<NOMAD::Double>("SPECULATIVE_SEARCH_BASE_FACTOR");
    }
}


void NOMAD::SpeculativeSearchMethod::generateTrialPointsFinal()
{
    if (nullptr == _iterAncestor)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"SpeculativeSearchMethod: must have an iteration ancestor");
    }

    auto barrier = getMegaIterationBarrier();

    if (nullptr == barrier)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"SpeculativeSearchMethod needs a barrier");
    }



    // Generate points starting from all points in the barrier.
    // If FRAME_CENTER_USE_CACHE is false (default), that is the same
    // as using the best feasible and best infeasible points.
    std::vector<NOMAD::EvalPoint> frameCenters;
    
    auto firstXIncFeas = barrier->getCurrentIncumbentFeas();
    auto firstXIncInf  = barrier->getCurrentIncumbentInf();
    if (firstXIncFeas)
    {
        frameCenters.push_back(*firstXIncFeas);
    }
    if (firstXIncInf)
    {
        frameCenters.push_back(*firstXIncInf);
    }
    
    
    for (const auto & frameCenter : frameCenters)
    {
        bool canGenerate = true;
        // Test that the frame center has a valid generating direction
        auto pointFrom = frameCenter.getPointFrom(NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
        if (nullptr == pointFrom || *pointFrom == frameCenter)
        {
            canGenerate = false;
        }

        if (canGenerate)
        {
            // Note: Recomputing direction, instead of using frameCenter.getDirection(),
            // to ensure we work in subspace.
            auto dir = NOMAD::Point::vectorize(*pointFrom, frameCenter);

            OUTPUT_INFO_START
            AddOutputInfo("Frame center: " + frameCenter.display());
            AddOutputInfo("Direction before scaling: " + dir.display());
            OUTPUT_INFO_END

            
            if (_nbSearches == NOMAD::INF_SIZE_T)
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"SpeculativeSearchMethod: can not have INF for SPECULATIVE_SEARCH_MAX.");
            }
            for (size_t i = 1; i <= _nbSearches; i++)
            {
                auto diri = dir;
                for(size_t j = 0 ; j < dir.size(); j++)
                {
                    diri[j] *= _baseFactor * (double)i;
                }

                OUTPUT_INFO_START
                AddOutputInfo("Scaled direction : " + diri.display());
                OUTPUT_INFO_END

                // Generate
                auto evalPoint = NOMAD::EvalPoint(*(pointFrom->getX()) + diri);

                // Insert the point
                evalPoint.setPointFrom(std::make_shared<NOMAD::EvalPoint>(frameCenter), NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
                evalPoint.addGenStep(getStepType());
                insertTrialPoint(evalPoint);
            }
        }
    }
}
