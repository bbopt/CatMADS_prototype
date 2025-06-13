
#include "../Algos/Algorithm.hpp"
#include "../Algos/EvcInterface.hpp"
#include "../Algos/Termination.hpp"
#include "../Util/Clock.hpp"

void NOMAD::Termination::init()
{
    setStepType(NOMAD::StepType::TERMINATION);
    verifyParentNotNull();
    
    if (nullptr != _runParams)
    {
        _maxIterations = _runParams->getTypeAttribute<size_t>("MAX_ITERATIONS");
        _maxTime = _runParams->getTypeAttribute<size_t>("MAX_TIME");
        
        _stopIfFeasible = _runParams->getTypeAttribute<bool>("STOP_IF_FEASIBLE");
        
        _stopIfPhaseOneSolution = _runParams->getTypeAttribute<bool>("STOP_IF_PHASE_ONE_SOLUTION");
    }
    
}

bool NOMAD::Termination::runImp()
{
    return _stopReasons->checkTerminate() ;
}


bool NOMAD::Termination::terminate(size_t iteration)
{
    bool stop = _stopReasons->checkTerminate();
    if (stop)
    {
        // A stop condition was already reached.
        return stop;
    }

    // Set stopReason due to criteria other than AlgoStopReasons<>

    // Compute type for testing feasibility
    auto computeTypeS = NOMAD::EvcInterface::getEvaluatorControl()->getFHComputeTypeS();
    auto evalType = NOMAD::EvcInterface::getEvaluatorControl()->getCurrentEvalType();
    NOMAD::FHComputeType computeType = {evalType, computeTypeS};
    
    // Termination conditions go here.
    // This is also tested in EvaluatorControl
    if (NOMAD::Step::getUserTerminate())
    {
        // Force quit (by pressing CTRL-C):
        _stopReasons->set(NOMAD::BaseStopType::CTRL_C);
    }
    else if (_maxIterations->getValue() < NOMAD::INF_SIZE_T && iteration > _maxIterations->getValue())
    {
        // Max iterations reached
        _stopReasons->set(NOMAD::IterStopType::MAX_ITER_REACHED);
    }
    else if (_maxTime->getValue() < NOMAD::INF_SIZE_T && NOMAD::Clock::getTimeSinceStart() >= _maxTime->getValue())
    {
        // Max time reached
        _stopReasons->set(NOMAD::BaseStopType::MAX_TIME_REACHED);
    }
    else if (_stopIfFeasible->getValue() && solHasFeas(computeType))
    {
        _stopReasons->set(NOMAD::IterStopType::STOP_ON_FEAS);
    }
    else if ( _stopIfPhaseOneSolution->getValue() && hasPhaseOneSolution())
    {
        _stopReasons->set(NOMAD::IterStopType::PHASE_ONE_COMPLETED);
    }
    else
    {
        // Need to check on MaxEval and MaxBBEval a last time because in evaluatorControl
        // the stop reason may have been set due to all queue points evaluated.
        stop = NOMAD::EvcInterface::getEvaluatorControl()->reachedMaxEval();
    }

    stop = stop || _stopReasons->checkTerminate() ;
    return stop;
}


void NOMAD::Termination::endImp()
{
    const NOMAD::Algorithm* currentAlgo = getParentOfType<NOMAD::Algorithm*>();
    NOMAD::OutputLevel outputLevel = currentAlgo->isSubAlgo() ? NOMAD::OutputLevel::LEVEL_INFO
                                                              : NOMAD::OutputLevel::LEVEL_HIGH;
    // Early out
    if ( ! NOMAD::OutputQueue::GoodLevel(outputLevel) )
    {
        return;
    }
    
    if (_stopReasons->checkTerminate())
    {
        std::string terminationInfo = "A termination criterion is reached: ";
        terminationInfo += _stopReasons->getStopReasonAsString();
        auto evc = NOMAD::EvcInterface::getEvaluatorControl();
        if (_stopReasons->testIf(NOMAD::EvalGlobalStopType::MAX_BB_EVAL_REACHED))
        {
            terminationInfo += " " + NOMAD::itos(evc->getBbEval());
        }
        else if (_stopReasons->testIf(NOMAD::EvalGlobalStopType::MAX_EVAL_REACHED))
        {
            terminationInfo += " " + NOMAD::itos(evc->getNbEval());
        }
        else if (_stopReasons->testIf(NOMAD::EvalGlobalStopType::MAX_BLOCK_EVAL_REACHED))
        {
            terminationInfo += " " + NOMAD::itos(evc->getBlockEval());
        }
        else if (evc->testIf(NOMAD::EvalMainThreadStopType::MAX_MODEL_EVAL_REACHED))
        {
            terminationInfo += " " + NOMAD::itos(evc->getTotalModelEval());
        }
        else if (evc->testIf(NOMAD::EvalMainThreadStopType::LAP_MAX_BB_EVAL_REACHED))
        {
            terminationInfo += " " + NOMAD::itos(evc->getLapBbEval());
        }
        AddOutputInfo(terminationInfo, outputLevel);
    }
    else
    {
        std::string terminationInfo = "No termination criterion reached";
        AddOutputInfo(terminationInfo, outputLevel);
    }

}
