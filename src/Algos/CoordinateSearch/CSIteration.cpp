
#include <algorithm>    // For std::merge and std::unique

#include "../../Algos/CoordinateSearch/CSIteration.hpp"
#include "../../Output/OutputQueue.hpp"

//#ifdef TIME_STATS
//#include "../../Algos/EvcInterface.hpp"
//#include "../../Util/Clock.hpp"
//
//// Initialize static variables
//double NOMAD::CSIteration::_iterTime = 0.0;
//double NOMAD::CSIteration::_pollTime = 0.0;
//double NOMAD::CSIteration::_pollEvalTime = 0.0;
//#endif // TIME_STATS


void NOMAD::CSIteration::init()
{
    setStepType(NOMAD::StepType::ITERATION);
    
    if (nullptr != _runParams && _runParams->getAttributeValue<bool>("MEGA_SEARCH_POLL"))
    {
        throw NOMAD::Exception(__FILE__, __LINE__,"CS does not support Mega Search Poll. ");
    }
    else
    {
        _csPoll = std::make_unique<NOMAD::CSPoll>(this);
    }
    
}


void NOMAD::CSIteration::startImp()
{
//#ifdef TIME_STATS
//    _iterStartTime = NOMAD::Clock::getCPUTime();
//#endif // TIME_STATS
}


bool NOMAD::CSIteration::runImp()
{
    bool iterationSuccess = false;
    
    OUTPUT_INFO_START
    std::string s = "No search method called by Coordinate Search algorithm.";
    AddOutputDebug(s);
    OUTPUT_INFO_END
    
    if ( ! _stopReasons->checkTerminate() )
    {
//#ifdef TIME_STATS
//        double pollStartTime = NOMAD::Clock::getCPUTime();
//        double pollEvalStartTime = NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime();
//#endif // TIME_STATS
        // 2. CS Poll
        _csPoll->start();
        // Iteration is a success if either a better xFeas or
        // a better xInf (partial success or dominating) xInf was found.
        // See Algorithm 12.2 from DFBO.
        iterationSuccess = _csPoll->run();
        _csPoll->end();
        
//#ifdef TIME_STATS
//        _pollTime += NOMAD::Clock::getCPUTime() - pollStartTime;
//        _pollEvalTime += NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime() - pollEvalStartTime;
//#endif // TIME_STATS
    }
    
    // End of the iteration: iterationSuccess is true iff we have a full success.
    return iterationSuccess;
}


//#ifdef TIME_STATS
//void NOMAD::CSIteration::endImp()
//{
//    _iterTime += NOMAD::Clock::getCPUTime() - _iterStartTime;
//}
//#endif // TIME_STATS



