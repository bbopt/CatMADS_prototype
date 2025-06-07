/**
 \file   DiscoMadsIteration.cpp
 \brief  The DiscoMads algorithm main iteration: implementation
 \author Solene Kojtych
 \see    DiscoMadsIteration.hpp
 */
#include <algorithm>    // For std::merge and std::unique
#include "../../nomad_platform.hpp"
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/DiscoMads/DiscoMadsIteration.hpp"
#include "../../Algos/DiscoMads/RevealingPoll.hpp"

#ifdef TIME_STATS
#include "../../Util/Clock.hpp"
#endif // TIME_STATS

void NOMAD::DiscoMadsIteration::init()
{
    // Initialize revealing poll for discoMads
    // For some testing, it is possible that _runParams is null
    if (nullptr == _runParams || !_runParams->getAttributeValue<bool>("MEGA_SEARCH_POLL"))
    {
        _revealingPoll = std::make_unique<NOMAD::RevealingPoll>(this);
    }
}


bool NOMAD::DiscoMadsIteration::runImp()
{
    bool iterationSuccess = false;
    
    // Parameter Update is handled at the upper level - MegaIteration.
    if ( nullptr != _megasearchpoll
        && !_stopReasons->checkTerminate())
    {
        _megasearchpoll->start();
        bool successful = _megasearchpoll->run();
        _megasearchpoll->end();

        if (successful)
        {
            OUTPUT_DEBUG_START
            std::string s = getName() + ": new success " + NOMAD::enumStr(_success);
            s += " stopReason = " + _stopReasons->getStopReasonAsString() ;
            AddOutputDebug(s);
            OUTPUT_DEBUG_END
        }
    }
    else
    {
        // 1. Search
        if ( nullptr != _search && ! _stopReasons->checkTerminate() )
        {
#ifdef TIME_STATS
            double searchStartTime = NOMAD::Clock::getCPUTime();
            double searchEvalStartTime = NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime();
#endif // TIME_STATS
        
            _search->start();
            iterationSuccess = _search->run();
            _search->end();
#ifdef TIME_STATS
            _searchTime += NOMAD::Clock::getCPUTime() - searchStartTime;
            _searchEvalTime += NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime() - searchEvalStartTime;
#endif // TIME_STATS

        }

        if ( nullptr != _search && ! _stopReasons->checkTerminate() )
        {
            if (iterationSuccess)
            {
                OUTPUT_INFO_START
                AddOutputInfo("Search Successful. Enlarge Delta frame size.");
                OUTPUT_INFO_END
            }
            else
            {
#ifdef TIME_STATS
                double pollStartTime = NOMAD::Clock::getCPUTime();
                double pollEvalStartTime = NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime();
#endif // TIME_STATS
                // 2. Revealing Poll
                _revealingPoll->start();
                iterationSuccess = _revealingPoll->run();
                _revealingPoll->end();
 
                if (iterationSuccess)
                {
                    OUTPUT_INFO_START
                    AddOutputInfo("Revealing Poll Successful (full success).");
                    OUTPUT_INFO_END
                }
                
                // 3. Mads Poll
                     // need to call checkTerminate in case of revelation during the revealing poll (then evaluations are stopped)
                else if(! _stopReasons->checkTerminate()){   
                    _poll->start();
                    // Iteration is a success if either a better xFeas or
                    // a better xInf (partial success or dominating) xInf was found.
                    // See Algorithm 12.2 from DFBO.
                    iterationSuccess = _poll->run();
                    _poll->end();
                }

#ifdef TIME_STATS
                _pollTime += NOMAD::Clock::getCPUTime() - pollStartTime;
                _pollEvalTime += NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime() - pollEvalStartTime;
#endif // TIME_STATS
            }
        }
    }


    // End of the iteration: iterationSuccess is true iff we have a full success.
    return iterationSuccess;
}
