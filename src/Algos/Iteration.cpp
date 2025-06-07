
#include <algorithm>    // For std::merge and std::unique

#include "../Algos/Iteration.hpp"
#include "../Output/OutputQueue.hpp"
#include "../Type/CallbackType.hpp"


NOMAD::Iteration::~Iteration()
{
    NOMAD::OutputQueue::Flush();
}


void NOMAD::Iteration::init()
{
    setStepType(NOMAD::StepType::ITERATION);
    verifyParentNotNull();
    
    _userCallbackEnabled = false;
    if (nullptr != _runParams)
    {
        _userCallbackEnabled = _runParams->getAttributeValue<bool>("USER_CALLS_ENABLED");
    }
}


std::string NOMAD::Iteration::getName() const
{
    return getAlgoName() + NOMAD::stepTypeToString(_stepType) + " " + std::to_string(_k);
}


void NOMAD::Iteration::endImp()
{
    OUTPUT_INFO_START
    AddOutputInfo("Stop reason: " + _stopReasons->getStopReasonAsString() );
    OUTPUT_INFO_END
    if ( _userCallbackEnabled )
    {
        bool stop = false;

        // Callback user provided function to check if user requested a stop.
        runCallback(NOMAD::CallbackType::ITERATION_END, *this, stop);
        if (!_stopReasons->checkTerminate() && stop)
        {
            _stopReasons->set(NOMAD::BaseStopType::USER_GLOBAL_STOP);
        }
        
        // Reset user iteration stop reason
        if (_stopReasons->testIf(NOMAD::IterStopType::USER_ITER_STOP))
        {
            _stopReasons->set(NOMAD::IterStopType::STARTED);
        }
    }
}




