
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/Mads/QuadSearchMethod.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Output/OutputQueue.hpp"

#ifdef USE_SGTELIB
#include "../../Algos/QuadModel/QuadModelSinglePass.hpp"
#endif

//
// Reference: File Sgtelib_Model_Search.cpp in NOMAD 3.9.1
// Author: Bastien Talgorn

void NOMAD::QuadSearchMethod::init()
{
    setStepType(NOMAD::StepType::SEARCH_METHOD_QUAD_MODEL);
    verifyParentNotNull();

    const auto parentSearch = getParentStep()->getParentOfType<NOMAD::QuadSearchMethod*>(false);

    // For some testing, it is possible that _runParams is null or evaluator control is null
    setEnabled((nullptr == parentSearch)
               && (nullptr !=_runParams)
               && (_runParams->getAttributeValue<bool>("QUAD_MODEL_SEARCH") || _runParams->getAttributeValue<bool>("QUAD_MODEL_SEARCH_SIMPLE_MADS"))
               &&  (nullptr != EvcInterface::getEvaluatorControl()));
#ifndef USE_SGTELIB
    if (isEnabled())
    {
        OUTPUT_INFO_START
        AddOutputInfo(getName() + " cannot be performed because NOMAD is compiled without sgtelib library");
        OUTPUT_INFO_END
        setEnabled(false);
    }
#endif

#ifdef USE_SGTELIB
    // Check that there is exactly one objective
    if (isEnabled())
    {
        auto nbObj = NOMAD::Algorithm::getNbObj();
        if (0 == nbObj)
        {
            OUTPUT_INFO_START
            AddOutputInfo(getName() + " not performed when there is no objective function");
            OUTPUT_INFO_END
            setEnabled(false);
        }
        else if (nbObj > 1)
        {
            OUTPUT_INFO_START
            AddOutputInfo(getName() + " not performed on multi-objective function");
            OUTPUT_INFO_END
            setEnabled(false);
        }

        const auto& modelDisplay = _runParams->getAttributeValue<std::string>("QUAD_MODEL_DISPLAY");
        _displayLevel = modelDisplay.empty()
                            ? NOMAD::OutputLevel::LEVEL_DEBUGDEBUG
                            : NOMAD::OutputLevel::LEVEL_INFO;
        
        
        _dynamicSearch = _runParams->getAttributeValue<bool>("QUAD_MODEL_SEARCH_DYNAMIC_ENABLE");
        // The first search is dynamic-enabled by default. After that, it is set in updateDynamicEnabled()
    }
    
#endif
}

void NOMAD::QuadSearchMethod::updateDynamicEnabled()
{
    // After a full success we rarely have another one. Let's deactivate for the next iteration
    _dynamicSearchEnabled = !(_allSuccessTypes.back() == NOMAD::SuccessType::FULL_SUCCESS);
    return;

    
// A more complex use of stats to enable or not the search
// TESTED-> not has good as what is done above
//    const size_t maxLastST = 5;
//
//    if (_allSuccessTypes.size() < maxLastST)
//    {
//        _dynamicSearchEnabled = true;
//        return ;
//    }
//
//    // After a full success we rarely have another one
//    if (_allSuccessTypes.back() == NOMAD::SuccessType::FULL_SUCCESS)
//    {
//        _dynamicSearchEnabled = false;
//        return;
//    }
//
//    std::list<NOMAD::SuccessType>::iterator it = _allSuccessTypes.end();
//    std::advance (it,-maxLastST);
//
//    // Iterator position to look into the last success types
//    size_t nbSucc = std::count(it, _allSuccessTypes.end(), NOMAD::SuccessType::FULL_SUCCESS) +
//            std::count(it, _allSuccessTypes.end(), NOMAD::SuccessType::PARTIAL_SUCCESS);
//
//    size_t nbProductiveCallsInLast = maxLastST
//                          - std::count(it, _allSuccessTypes.end(), NOMAD::SuccessType::UNDEFINED) // If SuccessType::UNDEFINED, search method has not been called.
//                          - std::count(it, _allSuccessTypes.end(), NOMAD::SuccessType::NO_TRIALS); // search method may have been called but no trial points producted.
//
//    if (nbProductiveCallsInLast >= 3 && nbSucc == 0)
//    {
//        _dynamicSearchEnabled = false;
//    }
//    else
//    {
//        _dynamicSearchEnabled = true;
//    }
    
    
}


void NOMAD::QuadSearchMethod::generateTrialPointsFinal()
{
#ifdef USE_SGTELIB
    // The trial points are generated for a feasible frame center and an infeasible one.
    if ( ! _stopReasons->checkTerminate() )
    {
        auto madsIteration = getParentOfType<MadsIteration*>();

        // MegaIteration's barrier member is already in sub dimension.
        auto bestXIncFeas = madsIteration->getMegaIterationBarrier()->getCurrentIncumbentFeas();
        auto bestXIncInf  = madsIteration->getMegaIterationBarrier()->getCurrentIncumbentInf();
        const auto& computeType = madsIteration->getMegaIterationBarrier()->getFHComputeType();
        if (nullptr != bestXIncFeas
            && bestXIncFeas->getF(computeType).isDefined()
            && bestXIncFeas->getF(computeType) < MODEL_MAX_OUTPUT)
        {
            NOMAD::QuadModelSinglePass singlePassFeas(this, bestXIncFeas, madsIteration->getMesh(),{} /* no scaling direction */);

            // Generate the trial points
            singlePassFeas.generateTrialPoints();

            // Pass the generated trial pts to this
            const auto& trialPtsSinglePassFeas = singlePassFeas.getTrialPoints();
            for (auto evalPoint : trialPtsSinglePassFeas)
            {
                evalPoint.setPointFrom(bestXIncFeas, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
                evalPoint.addGenStep(getStepType(), false /*do not inherit -> just qms step*/);
                insertTrialPoint(evalPoint);
            }
        }
        if (nullptr != bestXIncInf
            && bestXIncInf->getF(computeType).isDefined()
            && bestXIncInf->getF(computeType) < MODEL_MAX_OUTPUT
            && bestXIncInf->getH(computeType).isDefined()
            && bestXIncInf->getH(computeType) < MODEL_MAX_OUTPUT)
        {
            NOMAD::QuadModelSinglePass singlePassInf(this, bestXIncInf, madsIteration->getMesh(),{} /* no scaling direction */);

            // Generate the trial points
            singlePassInf.generateTrialPoints();

            // Pass the generated trial pts to this
            const auto& trialPtsSinglePassInf = singlePassInf.getTrialPoints();
            for (auto evalPoint : trialPtsSinglePassInf)
            {
                evalPoint.setPointFrom(bestXIncInf, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
                evalPoint.addGenStep(getStepType(), false /*do not inherit-> just qms step */);
                insertTrialPoint(evalPoint);
            }
        }
    }
#endif
}
 // end generateTrialPoints
