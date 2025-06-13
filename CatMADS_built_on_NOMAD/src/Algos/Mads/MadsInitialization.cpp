
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/CacheInterface.hpp"
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"
#include "../../Algos/DiscoMads/DiscoMadsBarrier.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Algos/Mads/GMesh.hpp"
#include "../../Algos/Mads/MadsInitialization.hpp"
#include "../../Algos/PhaseOne/PhaseOne.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Eval/ProgressiveBarrier.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Util/MicroSleep.hpp"

void NOMAD::MadsInitialization::init()
{
    _initialMesh = std::make_shared<NOMAD::GMesh>(_pbParams,_runParams);

    _bbInputType = _pbParams->getAttributeValue<NOMAD::BBInputTypeList>("BB_INPUT_TYPE");

    _hMax0 = _runParams->getAttributeValue<NOMAD::Double>("H_MAX_0");
}


bool NOMAD::MadsInitialization::runImp()
{

    bool doContinue = ! _stopReasons->checkTerminate();

    if (doContinue)
    {
        eval_x0s();
        doContinue = ! _stopReasons->checkTerminate();
    }

    return doContinue;
}


// TODO consider using IterationUtils insert and eval (should derive from it)
// Not obvious it is possible because of the cache.
bool NOMAD::MadsInitialization::eval_x0s()
{
    bool evalOk = false;
    std::string s;

    validateX0s();

    // Add X0s that need evaluation to eval queue
    NOMAD::CacheInterface cacheInterface(this);
    NOMAD::EvcInterface evcInterface(this);
    auto evc = evcInterface.getEvaluatorControl();

    // The compute type info to initialize the barrier is stored into the evaluator control
    auto evalType = NOMAD::EvalType::BB;
    FHComputeTypeS computeType /* default initializer used */;
    if (nullptr != evc)
    {
        evalType = evc->getCurrentEvalType();
        computeType = evc->getFHComputeTypeS();
        evc->lockQueue();
    }

    NOMAD::EvalPointSet evalPointSet;
    for (auto& x0 : _x0s)
    {
        NOMAD::EvalPoint evalPointX0(x0);

        // Create a trial point with a tag
        evalPointX0.updateTag();

        evalPointSet.insert(evalPointX0);
    }
    _trialPointStats.incrementTrialPointsGenerated(evalPointSet.size(), evalType);

    // Add points to the eval queue.
    // Convert to full dimension if needed.
    // Note: Queue is already locked - it needs to be locked to add points.
    evcInterface.keepPointsThatNeedEval(evalPointSet, false);   // false: no mesh

    if (nullptr != evc)
    {
        // Enforce no opportunism.
        auto previousOpportunism = evc->getOpportunisticEval();
        evc->setOpportunisticEval(false);
        evc->unlockQueue(false); // false: do not sort eval queue

        // Evaluate all x0s. Ignore returned success type.
        // Note: EvaluatorControl would not be able to compare/compute success since there is no barrier.
        evcInterface.startEvaluation();

        // Reset opportunism to previous values.
        evc->setOpportunisticEval(previousOpportunism);
    }

    bool x0Failed = true;

    // Construct barrier using points evaluated by this step.
    // The points are cleared from the EvaluatorControl.
    auto evaluatedPoints = evcInterface.retrieveAllEvaluatedPoints();
    std::vector<NOMAD::EvalPoint> evalPointX0s;

    for (const auto & x0 : _x0s)
    {
        NOMAD::EvalPoint evalPointX0(x0);

        // Look for x0 in freshly evaluated points
        bool x0Found = findInList(x0, evaluatedPoints, evalPointX0);

        if (!x0Found)
        {
            // Done stop waiting in cache.
            NOMAD::CacheBase::getInstance()->setStopWaiting(true);

            // Look for x0 in cache
            x0Found = (cacheInterface.find(x0, evalPointX0, evalType) > 0);

        }

        evalPointX0s.push_back(evalPointX0);
        if (x0Found && evalPointX0.isEvalOk(evalType))
        {
            // evalOk is true if at least one evaluation is Ok
            evalOk = true;

            x0Failed = false;   // At least one good X0.
        }
    }


    if (x0Failed)
    {
        // All x0s failed. Show an error.
        auto madsStopReason = NOMAD::AlgoStopReasons<NOMAD::MadsStopType>::get(_stopReasons);
        madsStopReason->set(NOMAD::MadsStopType::X0_FAIL);

        // If X0 fails, initialization is unsuccessful.
        _success = NOMAD::SuccessType::UNSUCCESSFUL;

        for (const auto & ep: evalPointX0s)
        {
            const auto x0Full = ep.getX()->makeFullSpacePointFromFixed(NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
            AddOutputError("Evaluation failed for X0 = " + x0Full.display() + " -> Raw bb outputs obtained: \"" + ep.getBBO(evalType) + "\"");
            const auto eval = ep.getEval(evalType);
            if (eval != nullptr && !eval->getBBOutputTypeList().empty() && eval->getBBOutput().getBBOAsArrayOfDouble().size() != eval->getBBOutputTypeList().size())
            {
                AddOutputError("Evaluation of point do not return the registered number of outputs, " +  std::to_string(ep.getEval(evalType)->getBBOutput().getBBOAsArrayOfDouble().size()) + " instead of " + std::to_string(ep.getEval(evalType)->getBBOutputTypeList().size()) + " expected. You may need to increase the buffer size in $NOMAD_HOME/src/Util/defines.hpp and rebuild Nomad." );
            }
        }
    }
    else
    {
        _trialPointStats.incrementEvalsDone(evalPointX0s.size(), evalType);

        // For initialization, any evaluated point that do not fail is a success.
        // TODO check what happen if cache is used.
        _success = NOMAD::SuccessType::FULL_SUCCESS;

        if (NOMAD::EvalType::BB == evalType)
        {
            // Several points can be successful
            _successStats.updateStats(evc->getSuccessStats());

            // After transfer from evaluator control make sure to reset stats
            evc->resetSuccessStats();

        }



        OUTPUT_INFO_START
        for (const auto & evalPointX0 : evalPointX0s)
        {
            s = "Using X0: ";
            // BB: Simple display. MODEL: Full display.
            s += (NOMAD::EvalType::BB == evalType) ? evalPointX0.display(computeType) : evalPointX0.displayAll(computeType);
        }
        AddOutputInfo(s);
        OUTPUT_INFO_END


        // Construct barrier using x0s (can use cache if option is enabled during constructor)
        if (_isUsedForDMultiMads)
        {

            // Force update of cache for mesh
            if (NOMAD::EvalType::BB == evalType)
            {
                for (auto & evalPointX0 : evalPointX0s)
                {
                    evalPointX0.setMesh(_initialMesh);
                }
            }
            const size_t incumbentSelectionThreshold = _runParams->getAttributeValue<size_t>("DMULTIMADS_SELECT_INCUMBENT_THRESHOLD");

            _barrier = std::make_shared<NOMAD::DMultiMadsBarrier>(
                                    NOMAD::Algorithm::getNbObj(),
                                    _hMax0,
                                    incumbentSelectionThreshold,
                                    NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this),
                                    evalType,
                                    computeType,
                                    evalPointX0s,
                                    false, /*  barrier NOT initialized from Cache */
                                    _bbInputType);

        }
        else if(_isUsedForDiscoMads)
        {
            _barrier = std::make_shared<NOMAD::DiscoMadsBarrier>(_hMax0,
                                                    NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this),
                                                    evalType,
                                                    computeType,
                                                    evalPointX0s,
                                                    _barrierInitializedFromCache,
                                                    _runParams->getAttributeValue<Double>("DISCO_MADS_EXCLUSION_RADIUS")
                                                   );
        }
        else
        {
            _barrier = std::make_shared<NOMAD::ProgressiveBarrier>(_hMax0,
                                                    NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this),
                                                    evalType,
                                                    computeType,
                                                    evalPointX0s,
                                                    _barrierInitializedFromCache);
        }

        // Case where x0 evaluation does not satisfy an extreme barrier constraint
        if (nullptr == _barrier->getCurrentIncumbentFeas() && nullptr == _barrier->getCurrentIncumbentInf())
        {
            // Run PhaseOne, which has its own Mads.
            // Then continue regular Mads with an initial feasible point (if found by phaseOne).

            auto phaseOneStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::PhaseOneStopType>>();
            auto phaseOne = std::make_shared<NOMAD::PhaseOne>(this,
                                                              phaseOneStopReasons,
                                                              _runParams,
                                                              _pbParams);
            // Ensure PhaseOne does not show found solutions
            phaseOne->setEndDisplay(false);

            phaseOne->start();
            bool success = phaseOne->run();
            phaseOne->end();


            if (!success || phaseOneStopReasons->checkTerminate() )
            {
                auto madsStopReason = NOMAD::AlgoStopReasons<NOMAD::MadsStopType>::get(_stopReasons);
                madsStopReason->set(NOMAD::MadsStopType::PONE_SEARCH_FAILED);
            }
            else
            {
                // Pass POne barrier point(s) to Mads barrier
                auto pOneBarrierPoints = phaseOne->getRefMegaIteration()->getBarrier()->getAllPoints();
                _barrier->updateWithPoints(pOneBarrierPoints, false , true /* true: update barrier incumbents and hMax */);
            }
        }
    }

    NOMAD::OutputQueue::Flush();

    if (_stopReasons->checkTerminate())
    {
        return false;
    }

    return evalOk;
}
