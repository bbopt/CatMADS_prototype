
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/Mads/ExtendedPoll.hpp"
#include "../../Output/OutputQueue.hpp"

#ifdef TIME_STATS
#include "../../Util/Clock.hpp"

// Initialize static variables
double NOMAD::ExtendedPoll::_extendedPollTime=0.0;
double NOMAD::ExtendedPoll::_extendedPollEvalTime=0.0;
#endif // TIME_STATS

void NOMAD::ExtendedPoll::init()
{
    setStepType(NOMAD::StepType::EXTENDED_POLL);
    verifyParentNotNull();
    
    
    auto mads = dynamic_cast<const NOMAD::Mads*>(_iterAncestor->getFirstAlgorithm());
    
    _extendedPollMethod = mads->getExtendedPollMethod();
    
    if (nullptr == _extendedPollMethod)
    {
        _isEnabled = false;
        return;
    }
    _isEnabled = _extendedPollMethod->isEnabled();
    
    if (_isEnabled)
    {
        _extendedPollMethod->resetIterAncestor(_iterAncestor);
    }
    
}


void NOMAD::ExtendedPoll::startImp()
{
   // Sanity check.
    verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, false);

    // Reset the current trial stats before run is called and increment the number of calls
    // This is managed by iteration utils when using generateTrialPoint instead of the (start, run, end) sequence.
    _trialPointStats.resetCurrentStats();
    _trialPointStats.incrementNbCalls();
    
}


bool NOMAD::ExtendedPoll::runImp()
{
    bool extendedPollSuccessful = false;
    std::string s;
    
    // Sanity check. The runImp function should be called only when trial points are generated and evaluated for each search method separately.
    verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, false);
    
    if (!_isEnabled)
    {
        // Early out - Return false: No new success found.
        OUTPUT_DEBUG_START
        AddOutputDebug("Extended poll method not enabled.");
        OUTPUT_DEBUG_END
        return false;
    }
    
    // A local user stop is requested. Do not perform remaining search methods. Stop type reset is done at the end of iteration/megaiteration and algorithm.
    if (_stopReasons->testIf(NOMAD::IterStopType::USER_ITER_STOP) || _stopReasons->testIf(NOMAD::IterStopType::USER_ALGO_STOP) ||
        _stopReasons->testIf(NOMAD::EvalGlobalStopType::CUSTOM_GLOBAL_STOP)) // C.T : I think we should test NOMAD::EvalMainThreadStopType::CUSTOM_OPPORTUNISTIC_STOP as the others are not yet triggered
    {
        return false;
    }
    
    
    
#ifdef TIME_STATS
    double extendedPollStartTime = NOMAD::Clock::getCPUTime();
    double extendedPollEvalStartTime = NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime();
#endif // TIME_STATS
    
    _extendedPollMethod->start();
    _extendedPollMethod->run();
    _extendedPollMethod->end();
    
#ifdef TIME_STATS
    _extendedPollTime += NOMAD::Clock::getCPUTime() - extendedPollStartTime;
    _extendedPollEvalTime += NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime() - extendedPollEvalStartTime;
#endif // TIME_STATS
    
    // Manually set the SuccessType from _extendedPollMethod to ExtendedPoll
    setSuccessType(_extendedPollMethod->getSuccessType());

    // Search is successful only if full success type.
    extendedPollSuccessful = (_extendedPollMethod->getSuccessType() >= NOMAD::SuccessType::FULL_SUCCESS);
    if (extendedPollSuccessful)
    {
        // Do not go through the other search methods if a search is
        // successful.
        OUTPUT_INFO_START
        s = _extendedPollMethod->getName();
        s += " is successful. Stop reason: ";
        s += _stopReasons->getStopReasonAsString() ;
        
        AddOutputInfo(s);
        OUTPUT_INFO_END
    }
    
    
    return extendedPollSuccessful;
}


void NOMAD::ExtendedPoll::endImp()
{
    // Sanity check. The endImp function should be called only when trial points are generated and evaluated for extended poll method separately.
    verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, false);
    
    if (!_isEnabled)
    {
        // Early out
        return;
    }
    
    // Update the trial stats of the parent (Mads)
    // This is directly managed by iteration utils when using generateTrialPoint instead of the (start, run, end) sequence done here.
    _trialPointStats.updateParentStats();


    // Need to reset the EvalStopReason if a sub optimization is used during Search and the max bb is reached for this sub optimization
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    if (evc->testIf(NOMAD::EvalMainThreadStopType::LAP_MAX_BB_EVAL_REACHED))
    {
        evc->setStopReason(NOMAD::getThreadNum(), NOMAD::EvalMainThreadStopType::STARTED);
    }

}

