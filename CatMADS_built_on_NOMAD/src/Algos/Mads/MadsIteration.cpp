
#include <algorithm>    // For std::merge and std::unique

#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/Mads/UserSearchMethod.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Output/OutputQueue.hpp"

#ifdef TIME_STATS
#include "../../Algos/EvcInterface.hpp"
#include "../../Util/Clock.hpp"

// Initialize static variables
double NOMAD::MadsIteration::_iterTime = 0.0;
double NOMAD::MadsIteration::_searchTime = 0.0;
double NOMAD::MadsIteration::_searchEvalTime = 0.0;
double NOMAD::MadsIteration::_pollTime = 0.0;
double NOMAD::MadsIteration::_pollEvalTime = 0.0;
#endif // TIME_STATS


void NOMAD::MadsIteration::init()
{

    // For some testing, it is possible that _runParams is null
    if (nullptr != _runParams && _runParams->getAttributeValue<bool>("MEGA_SEARCH_POLL"))
    {
        _megasearchpoll = std::make_unique<NOMAD::MegaSearchPoll>(this);
    }
    else
    {
        _poll = std::make_unique<NOMAD::Poll>(this, _userCallbackEnabled);
        _search = std::make_unique<NOMAD::Search>(this);

        // Search method for user. Put it in first position.
        // The user search method is enabled by setting a
        // callback function and having parameter USER_SEARCH true.
        // The callback function is used to generate directions.
        // Trial points on mesh are produced automatically.
        if (_userCallbackEnabled)
        {
            auto userSearch = std::make_shared<NOMAD::UserSearchMethod>(this,1);
            _search->insertSearchMethod(0,userSearch);
            // ChT Temp for CatMads. The second user search is not required once
            // CatMads algo is available
            auto userSearch_2 = std::make_shared<NOMAD::UserSearchMethod>(this,2);
            _search->insertSearchMethod(1,userSearch_2);
        }
        
        _extendedPoll = std::make_unique<NOMAD::ExtendedPoll>(this);
    }
}


void NOMAD::MadsIteration::startImp()
{

#ifdef TIME_STATS
    _iterStartTime = NOMAD::Clock::getCPUTime();
#endif // TIME_STATS
}


NOMAD::ArrayOfPoint NOMAD::MadsIteration::suggest()
{
    NOMAD::ArrayOfPoint xs;

    if (nullptr != _megasearchpoll)
    {
        OUTPUT_INFO_START
        AddOutputInfo("Mads Iteration Suggest. Mega Search Poll.");
        OUTPUT_INFO_END

        // suggest uses MegaSearchPoll for now
        _megasearchpoll->start();

        const auto& trialPoints = _megasearchpoll->getTrialPoints();

        for (const auto & trialPoint : trialPoints)
        {
            NOMAD::EvalPoint evalPointFound;

            // Suggested points are already in cache put not evaluated
            NOMAD::CacheBase::getInstance()->find(trialPoint, evalPointFound);
            if (!evalPointFound.ArrayOfDouble::isDefined())
            {
                throw NOMAD::Exception(__FILE__, __LINE__, "MadsIteration suggest, trial point should be in cache");
            }
            if (nullptr == evalPointFound.getEval(NOMAD::EvalType::BB) && nullptr == evalPointFound.getEval(NOMAD::EvalType::SURROGATE) )
            {
                // Do not suggest point already in vector
                if (std::find(xs.begin(), xs.end(), *trialPoint.getX() ) == xs.end() )
                {
                    xs.push_back(*trialPoint.getX());
                }
            }
        }
        _megasearchpoll->end();
    }
    else
    {
       throw NOMAD::Exception(__FILE__, __LINE__, "MadsIteration suggest only performs with MEGA_SEARCH_POLL enabled");
    }

    return xs;
}


bool NOMAD::MadsIteration::runImp()
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
                // 2. Poll
                _poll->start();
                // Iteration is a success if either a better xFeas or
                // a better xInf (partial success or dominating) xInf was found.
                // See Algorithm 12.2 from DFBO.
                iterationSuccess = _poll->run();
                _poll->end();
#ifdef TIME_STATS
                _pollTime += NOMAD::Clock::getCPUTime() - pollStartTime;
                _pollEvalTime += NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime() - pollEvalStartTime;
#endif // TIME_STATS
                
                if (!iterationSuccess && nullptr != _extendedPoll && _extendedPoll->isEnabled())
                {
                    OUTPUT_INFO_START
                    AddOutputInfo("Poll unsuccessful. Let's do an extended poll.");
                    OUTPUT_INFO_END
#ifdef TIME_STATS
                    double extendedPollStartTime = NOMAD::Clock::getCPUTime();
                    double extendedPollEvalStartTime = NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime();
#endif // TIME_STATS
                    // 3. ExtendedPoll
                    _extendedPoll->start();
                    iterationSuccess = _extendedPoll->run();
                    _extendedPoll->end();
#ifdef TIME_STATS
                    _extendedPollTime += NOMAD::Clock::getCPUTime() - extendedPollStartTime;
                    _extendedPollEvalTime += NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime() - extendedPollEvalStartTime;
#endif // TIME_STATS
                    
                }

                
            }
        }
    }
    // End of the iteration: iterationSuccess is true iff we have a full success.
    return iterationSuccess;
}

#ifdef TIME_STATS
void NOMAD::MadsIteration::endImp()
{
    _iterTime += NOMAD::Clock::getCPUTime() - _iterStartTime;
}
#endif // TIME_STATS
