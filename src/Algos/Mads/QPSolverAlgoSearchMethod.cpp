
#include "../../Algos/Mads/QPSolverAlgoSearchMethod.hpp"
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/QPSolverAlgo/QPSolverAlgoSinglePass.hpp"
#include "../../Algos/SubproblemManager.hpp"

void NOMAD::QPSolverAlgoSearchMethod::init()
{
    setStepType(NOMAD::StepType::SEARCH_METHOD_QUAD_MODEL);
    verifyParentNotNull();

    const auto parentSearch = getParentStep()->getParentOfType<NOMAD::QPSolverAlgoSearchMethod*>(false);

    // For some testing, it is possible that _runParams is null or evaluator control is null
    setEnabled((nullptr == parentSearch)
               && (nullptr !=_runParams)
               && _runParams->getAttributeValue<bool>("QP_SEARCH")
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
        
        
        _dynamicSearch = _runParams->getAttributeValue<bool>("QP_SEARCH_DYNAMIC_ENABLE");
        // The first search is dynamic enabled by default. After that, it is set in updateDynamicSearchEnabled()
    }
#endif
}

void NOMAD::QPSolverAlgoSearchMethod::updateDynamicEnabled()
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

void NOMAD::QPSolverAlgoSearchMethod::generateTrialPointsFinal()
{
#ifdef USE_SGTELIB
    // The trial points are generated for a feasible frame center and an infeasible one.
    if ( ! _stopReasons->checkTerminate() )
    {
        auto madsIteration = getParentOfType<MadsIteration*>();

        // MegaIteration's barrier member is already in sub dimension.
        auto bestXFeas = madsIteration->getMegaIterationBarrier()->getCurrentIncumbentFeas();
        auto bestXInf  = madsIteration->getMegaIterationBarrier()->getCurrentIncumbentInf();

        const auto& computeType = madsIteration->getMegaIterationBarrier()->getFHComputeType();
        if (nullptr != bestXFeas
            && bestXFeas->getF(computeType).isDefined()
            && bestXFeas->getF(computeType) < MODEL_MAX_OUTPUT)
        {
            NOMAD::QPSolverAlgoSinglePass singlePassFeas(this, bestXFeas, madsIteration->getMesh(),{} /* no scaling direction */);

            // Generate the trial points
            singlePassFeas.generateTrialPoints();

            // Pass the generated trial pts to this
            const auto& trialPtsSinglePassFeas = singlePassFeas.getTrialPoints();
            for (auto evalPoint : trialPtsSinglePassFeas)
            {
                evalPoint.setPointFrom(bestXFeas, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
                insertTrialPoint(evalPoint);
            }
        }
        if (nullptr != bestXInf
            && bestXInf->getF(computeType).isDefined()
            && bestXInf->getF(computeType) < MODEL_MAX_OUTPUT
            && bestXInf->getH(computeType).isDefined()
            && bestXInf->getH(computeType) < MODEL_MAX_OUTPUT)
        {
            NOMAD::QPSolverAlgoSinglePass singlePassInf(this, bestXInf, madsIteration->getMesh(),{} /* no scaling direction */);

            // Generate the trial points
            singlePassInf.generateTrialPoints();

            // Pass the generated trial pts to this
            const auto& trialPtsSinglePassInf = singlePassInf.getTrialPoints();
            for (auto evalPoint : trialPtsSinglePassInf)
            {
                evalPoint.setPointFrom(bestXInf, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
                insertTrialPoint(evalPoint);
            }
        }
    }
#endif
}

