
#include <signal.h>

#include "../Algos/Algorithm.hpp"
#include "../Algos/EvcInterface.hpp"
#include "../Algos/Mads/SearchMethodBase.hpp"
#include "../Algos/SubproblemManager.hpp"
#include "../Cache/CacheBase.hpp"
#include "../Eval/ProgressiveBarrier.hpp"
#include "../Output/OutputQueue.hpp"
#include "../Math/RNG.hpp"
#include "../Util/fileutils.hpp"

#ifdef TIME_STATS
#include "../Util/Clock.hpp"
#endif // TIME_STATS

void NOMAD::Algorithm::init()
{

    // Verifications that throw Exceptions to the Constructor if not validated.
    verifyParentNotNull();

    if (nullptr == _runParams)
    {
        throw NOMAD::StepException(__FILE__, __LINE__,
                               "A valid RunParameters must be provided to an Algorithm constructor.", this);
    }

    if (nullptr == _pbParams)
    {
        throw NOMAD::StepException(__FILE__, __LINE__,
                               "A valid PbParameters must be provided to the Algorithm constructor.", this);
    }

    if ( nullptr == _stopReasons )
        throw NOMAD::StepException(__FILE__, __LINE__,
                               "Valid stop reasons must be provided to the Algorithm constructor.", this);

    // Is sub algo ?
    auto parentAlgo = getParentOfType<NOMAD::Algorithm*>();
    if (nullptr != parentAlgo)
    {
        _isSubAlgo = true;
    }
    
    // Check pbParams if needed, ex. if a copy of PbParameters was given to the Algorithm constructor.
    _pbParams->checkAndComply();

    // Instantiate generic algorithm termination
    _termination    = std::make_unique<NOMAD::Termination>( this, _runParams, _pbParams);

    // Update SubproblemManager
    // When the flag use only local variables is true, only the fixed variables given in _pbParams are considered. Otherwise, we use the subproblem manager to fetch the fixed variables from the parent pb.
    NOMAD::Point fullFixedVariable = (isRootAlgo()||_useOnlyLocalFixedVariables) ? _pbParams->getAttributeValue<NOMAD::Point>("FIXED_VARIABLE")
                                   : NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(_parentStep);

    NOMAD::Subproblem subproblem(_pbParams, fullFixedVariable);
    NOMAD::SubproblemManager::getInstance()->addSubproblem(this, subproblem);
    _pbParams = subproblem.getPbParams();
    _pbParams->checkAndComply();
    
    // Set some compute type and h norm type in evaluator control only if not sub algo
    // otherwise it is inherited.
    // Compute type in evaluator control can be used to initialized barrier compute type
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    if (! _isSubAlgo && nullptr != evc)
    {
        auto hNormType = _runParams->getAttributeValue<NOMAD::HNormType>("H_NORM");
        auto computeType = NOMAD::ComputeType::STANDARD; // Default compute type is set here. Override only by PhaseOne algo.
        evc->setHNormType(hNormType);
        evc->setComputeType(computeType);
    }

    /** Step::userInterrupt() will be called if CTRL-C is pressed.
     * Currently, the main thread will wait for all evaluations to be complete.
     * \todo Propagate interruption to all threads, for all parallel evaluations of blackbox.
     */
    signal(SIGINT, userInterrupt);

}


NOMAD::Algorithm::~Algorithm()
{
    NOMAD::SubproblemManager::getInstance()->removeSubproblem(this);
}


void NOMAD::Algorithm::startImp()
{

#ifdef TIME_STATS
    if (isRootAlgo())
    {
        _startTime = NOMAD::Clock::getCPUTime();
    }
#endif // TIME_STATS
    
    // Reset the current counters. The total counters are not reset (done only once when algo constructor is called).
    _trialPointStats.resetCurrentStats();
    
    
    // All stop reasons are reset.
    _stopReasons->setStarted();

    // Default success is reset
    // Success type is initialized in Step::defaultStart()
    _algoSuccessful = false;

    if (isRootAlgo())
    {
        // Update hot restart info
        readInformationForHotRestart();
        NOMAD::CacheBase::getInstance()->setStopWaiting(false);
    }

    // By default, reset the lap counter for BbEval and set the lap maxBbEval to INF
    NOMAD::EvcInterface::getEvaluatorControl()->resetLapBbEval();
    NOMAD::EvcInterface::getEvaluatorControl()->setLapMaxBbEval( NOMAD::INF_SIZE_T );
    NOMAD::EvcInterface::getEvaluatorControl()->resetModelEval();

    if (nullptr == _refMegaIteration)
    {
        // Default behavior - not hot restart.
        // Clear cache hits.
        // Initialization.
        // Eval X0s.

        if (isRootAlgo())
        {
            // Ensure we do not count cache hits which may have been read in the cache.
            NOMAD::CacheBase::resetNbCacheHits();
        }

        // Perform algo initialization only when available.
        if (nullptr != _initialization)
        {
            _initialization->start();
            _initialization->run();
            _initialization->end();
        }

    }
    else
    {
        // Hot restart situation.
        // We will not need Initialization.
        auto barrier = _refMegaIteration->getBarrier();

        // Update X0s
        // Use best points.
        auto bestPoints = barrier->getAllPoints();

        NOMAD::ArrayOfPoint x0s;
        if (!bestPoints.empty())
        {
            std::transform(bestPoints.begin(), bestPoints.end(), std::back_inserter(x0s),
                           [](const NOMAD::EvalPoint& evalPoint) -> NOMAD::EvalPoint { return evalPoint; });

        }
        _pbParams->setAttributeValue<NOMAD::ArrayOfPoint>("X0", std::move(x0s));
        _pbParams->checkAndComply();
    }
}


void NOMAD::Algorithm::endImp()
{
    
    if ( _endDisplay )
    {
        displayBestSolutions();
#ifdef TIME_STATS
        if (isRootAlgo())
        {
            _totalRealAlgoTime = NOMAD::Clock::getTimeSinceStart();
            _totalCPUAlgoTime += NOMAD::Clock::getCPUTime() - _startTime;
        }
#endif // TIME_STATS

        displayEvalCounts();
    }
    
    // Update parent if it exists (can be Algo or IterationUtils)  with this stats
    _trialPointStats.updateParentStats();

    // TODO work on that. It is not consistent with other info displayed in function displayEvalCounts() above;
    // Check example with display degree 4 and sub-optimization
//    OUTPUT_DEBUG_START
//    std::string s = "Total number of evals: " + std::to_string(_trialPointStats.getNbEvalsDone(NOMAD::EvalType::BB,true)) + "\n";
//    AddOutputDebug(s);
//    s = "Current number of evals: " + std::to_string(_trialPointStats.getNbEvalsDone(NOMAD::EvalType::BB,false)) + "\n";
//    AddOutputDebug(s);
//    if (EvcInterface::getEvaluatorControl()->hasEvaluator(NOMAD::EvalType::SURROGATE))
//    {
//        s = "Total number of surrogate evals: " + std::to_string(_trialPointStats.getNbEvalsDone(NOMAD::EvalType::SURROGATE,true)) + "\n";
//        AddOutputDebug(s);
//        s = "Current number of surrogate evals: " + std::to_string(_trialPointStats.getNbEvalsDone(NOMAD::EvalType::SURROGATE,false)) + "\n";
//        AddOutputDebug(s);
//    }
//    OUTPUT_DEBUG_END
 
    // Reset user algo stop reason
    if (_stopReasons->testIf(NOMAD::IterStopType::USER_ALGO_STOP))
    {
        _stopReasons->set(NOMAD::IterStopType::STARTED);
    }
    
    
    // Update the parent success
    Step * parentStep = const_cast<Step*>(_parentStep);
    parentStep->setSuccessType(_success);
    
    // By default, reset the lap counter for BbEval and set the lap maxBbEval to INF
    NOMAD::EvcInterface::getEvaluatorControl()->resetLapBbEval();
    NOMAD::EvcInterface::getEvaluatorControl()->setLapMaxBbEval( NOMAD::INF_SIZE_T );

    if (isRootAlgo())
    {
        saveInformationForHotRestart();
        NOMAD::CacheBase::getInstance()->setStopWaiting(true);
    }
}


void NOMAD::Algorithm::updateStats(TrialPointStats &trialPointStats)
{
    _trialPointStats.updateWithCurrentStats(trialPointStats);
}

void NOMAD::Algorithm::hotRestartOnUserInterrupt()
{
#ifdef TIME_STATS
    if (isRootAlgo())
    {
        _totalCPUAlgoTime += NOMAD::Clock::getCPUTime() - _startTime;
    }
#endif // TIME_STATS
    hotRestartBeginHelper();

    hotRestartEndHelper();
#ifdef TIME_STATS
    if (isRootAlgo())
    {
        _startTime = NOMAD::Clock::getCPUTime();
    }
#endif // TIME_STATS
}


void NOMAD::Algorithm::saveInformationForHotRestart() const
{
    // If we want to stop completely and then be able to restart
    // from where we were, we need to save some information on file.
    //
    // Issue 372: Maybe we need to write current parameters. If we write them,
    // ignore initial values, only take latest values down the Parameter tree.
    // For now, using initial parameters.

    // Cache file is treated independently of hot restart file.
    // As long as the cache file name is set, it is written.
    // This is the behavior of NOMAD 3.
    std::string cacheFile = NOMAD::CacheBase::getInstance()->getFileName();
    if (!cacheFile.empty())
    {
        NOMAD::CacheBase::getInstance()->write();
    }
    if ( _runParams->getAttributeValue<bool>("HOT_RESTART_WRITE_FILES"))
    {
        std::cout << "Save information for hot restart." << std::endl;
        std::cout << "Write hot restart file." << std::endl;
        NOMAD::write(*this, _runParams->getAttributeValue<std::string>("HOT_RESTART_FILE"));
    }
}


void NOMAD::Algorithm::displayBestSolutions() const
{
    std::vector<NOMAD::EvalPoint> evalPointList;
    // Display the best feasible solutions.
    std::string sFeas;
    // Output level is very high if there are no parent algorithm
    // Output level is info if this algorithm is a sub part of another algorithm.
    NOMAD::OutputLevel outputLevel = _isSubAlgo ? NOMAD::OutputLevel::LEVEL_INFO
                                                 : NOMAD::OutputLevel::LEVEL_VERY_HIGH;
    auto solFormat = NOMAD::OutputQueue::getInstance()->getSolFormat();
    
    // Complete compute type
    NOMAD::FHComputeTypeS computeType = NOMAD::EvcInterface::getEvaluatorControl()->getFHComputeTypeS();
    auto evalType = NOMAD::EvcInterface::getEvaluatorControl()->getCurrentEvalType();
    //auto hNormType = NOMAD::EvcInterface::getEvaluatorControl()->getHNormType();
    NOMAD::FHComputeType  completeComputeType = {evalType, computeType};
    
    auto surrogateAsBB = NOMAD::EvcInterface::getEvaluatorControl()->getSurrogateOptimization();
    if (isRootAlgo())
    {
        solFormat.set(-1);
    }
    NOMAD::OutputInfo displaySolFeas(getName(), sFeas, outputLevel);
    auto fixedVariable = NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this);

    sFeas = "Best feasible solution";
    auto barrier = getMegaIterationBarrier();
    
    // Let's try to build a progressive barrier from the cache
    if (nullptr == barrier)
    {
        barrier = std::make_shared<NOMAD::ProgressiveBarrier>(NOMAD::INF,
                                                              fixedVariable,
                                                              evalType,
                                                              computeType);
    }
    if (nullptr != barrier)
    {
        barrier->checkForFHComputeType(completeComputeType);
        for (auto const & p : barrier->getAllXFeas())
        {
            evalPointList.push_back(*p);
        }
        NOMAD::convertPointListToFull(evalPointList, fixedVariable);
    }
    size_t nbBestFeas = evalPointList.size();

    if (0 == nbBestFeas)
    {
        sFeas += ":     Undefined.";
        displaySolFeas.addMsg(sFeas);
    }
    else if (1 == nbBestFeas)
    {
        sFeas += ":     ";
        displaySolFeas.addMsg(sFeas + evalPointList[0].display(computeType,
                                                               solFormat,
                                                               NOMAD::DISPLAY_PRECISION_FULL,
                                                               surrogateAsBB));
    }
    else
    {
        sFeas += "s:    ";
        displaySolFeas.addMsg(sFeas + evalPointList[0].display(computeType,
                                                               solFormat,
                                                               NOMAD::DISPLAY_PRECISION_FULL,
                                                               surrogateAsBB));
    }


    const size_t maxSolCount = 8;
    if (nbBestFeas > 1)
    {
        std::vector<NOMAD::EvalPoint>::const_iterator it;
        size_t solCount = 0;
        for (it = evalPointList.begin(); it != evalPointList.end(); ++it)
        {
            solCount++;
            if (evalPointList.begin() == it)
            {
                continue;   // First element already added
            }
            sFeas = "                            ";
            displaySolFeas.addMsg(sFeas + it->display(computeType, solFormat,
                                                      NOMAD::DISPLAY_PRECISION_FULL,
                                                      surrogateAsBB));
            if (solCount >= maxSolCount)
            {
                // We printed enough solutions already.
                displaySolFeas.addMsg("... A total of " + std::to_string(evalPointList.size()) + " feasible solutions were found.");
                break;
            }
        }
    }

    NOMAD::OutputQueue::Add(std::move(displaySolFeas));

    evalPointList.clear();


    // Display the best infeasible solutions.
    std::string sInf;
    NOMAD::OutputInfo displaySolInf(getName(), sInf, outputLevel);
    sInf = "Best infeasible solution";
    if (nullptr != barrier)
    {
        for (auto const & p : barrier->getAllXInf())
        {
            evalPointList.push_back(*p);
        }
        NOMAD::convertPointListToFull(evalPointList, fixedVariable);
    }
    size_t nbBestInf = evalPointList.size();

    if (0 == nbBestInf)
    {
        sInf += ":   Undefined.";
        displaySolInf.addMsg(sInf);
    }
    else if (1 == nbBestInf)
    {
        sInf += ":   ";
        displaySolInf.addMsg(sInf + evalPointList[0].display(computeType,
                                                             solFormat,
                                                             NOMAD::DISPLAY_PRECISION_FULL,
                                                             surrogateAsBB));
    }
    else
    {
        sInf += "s:  ";
        displaySolInf.addMsg(sInf + evalPointList[0].display(computeType,
                                                             solFormat,
                                                             NOMAD::DISPLAY_PRECISION_FULL,
                                                             surrogateAsBB));
    }

    if (nbBestInf > 1)
    {
        size_t solCount = 0;
        std::vector<NOMAD::EvalPoint>::const_iterator it;
        for (it = evalPointList.begin(); it != evalPointList.end(); ++it)
        {
            solCount++;
            if (evalPointList.begin() == it)
            {
                continue;   // First element already added
            }
            displaySolInf.addMsg("                            " + it->display(computeType,
                                                                              solFormat,
                                                                              NOMAD::DISPLAY_PRECISION_FULL,
                                                                              surrogateAsBB));
            if (solCount >= maxSolCount)
            {
                // We printed enough solutions already.
                displaySolInf.addMsg("... A total of " + std::to_string(evalPointList.size()) + " infeasible solutions were found.");
                break;
            }
        }
    }

    NOMAD::OutputQueue::Add(std::move(displaySolInf));
}


void NOMAD::Algorithm::displayEvalCounts() const
{
    // Display evaluation information

    // _isSubAlgo is used to display or not certain values
    
    // Output levels will be modulated depending on the counts and on the Algorithm level.
    NOMAD::OutputLevel outputLevelHigh = _isSubAlgo ? NOMAD::OutputLevel::LEVEL_INFO
                                               : NOMAD::OutputLevel::LEVEL_HIGH;
    NOMAD::OutputLevel outputLevelNormal = _isSubAlgo ? NOMAD::OutputLevel::LEVEL_INFO
                                                 : NOMAD::OutputLevel::LEVEL_NORMAL;
    // Early out
    if ( ! NOMAD::OutputQueue::GoodLevel(outputLevelHigh) && ! NOMAD::OutputQueue::GoodLevel(outputLevelNormal) )
    {
        return;
    }

    // Actual numbers
    size_t bbEval       = NOMAD::EvcInterface::getEvaluatorControl()->getBbEval();
    size_t bbEvalFromCacheForRerun = NOMAD::EvcInterface::getEvaluatorControl()->getBbEvalFromCacheForRerun();
    size_t lapBbEval    = NOMAD::EvcInterface::getEvaluatorControl()->getLapBbEval();
    size_t nbEval       = NOMAD::EvcInterface::getEvaluatorControl()->getNbEval();
    size_t surrogateEval = NOMAD::EvcInterface::getEvaluatorControl()->getSurrogateEval();
    size_t surrogateEvalFromCacheForRerun = NOMAD::EvcInterface::getEvaluatorControl()->getSurrogateEvalFromCacheForRerun();
    size_t lapSurrogateEval= NOMAD::EvcInterface::getEvaluatorControl()->getLapSurrogateEval();
    size_t modelEval    = NOMAD::EvcInterface::getEvaluatorControl()->getModelEval();
    size_t totalModelEval = NOMAD::EvcInterface::getEvaluatorControl()->getTotalModelEval();
    size_t nbCacheHits  = NOMAD::CacheBase::getNbCacheHits();
    size_t nbRevealingIter = NOMAD::EvcInterface::getEvaluatorControl()-> getNbRevealingIter();
    int nbEvalNoCount   = static_cast<int>(nbEval - bbEval - nbCacheHits);

    // What needs to be shown, according to the counts and to the value of isSub
    bool showbbEvalFromCacheForRerun   = (bbEvalFromCacheForRerun > 0 );
    bool showNbEvalNoCount   = (nbEvalNoCount > 0);
    bool showModelEval       = _isSubAlgo && (modelEval > 0);
    bool showTotalModelEval  = (totalModelEval > 0);
    bool showNbCacheHits     = (nbCacheHits > 0);
    bool showNbEval          = (nbEval > bbEval);
    bool showLapBbEval       = _isSubAlgo && (bbEval > lapBbEval && lapBbEval > 0);
    bool showSurrogateEval   = (surrogateEval > 0);
    bool showSurrogateEvalFromCacheForRerun   = (surrogateEvalFromCacheForRerun > 0);
    bool showNbRevealingIter = nbRevealingIter>0;
    // bool showLapSurrogateEval= _isSubAlgo && (surrogateEval > lapSurrogateEval && lapSurrogateEval > 0);



    // Padding for nice presentation
    std::string sFeedBbEval, sFeedBbEvalFromCacheForRerun, sFeedLapBbEval, sFeedSurrogateEval, sFeedSurrogateEvalFromCacheForRerun, sFeedLapSurrogateEval, sFeedNbEvalNoCount, sFeedModelEval, sFeedTotalModelEval, sFeedCacheHits, sFeedNbEval, sFeedNbRevealingIter;

    // Conditional values: showNbEval, showNbEvalNoCount, showLapBbEval
    if (showbbEvalFromCacheForRerun)  // Longest title
    {
        sFeedBbEval += "           ";
        sFeedBbEvalFromCacheForRerun += "";
        //sFeedLapBbEval += "";
        sFeedNbEvalNoCount += "           ";
        sFeedModelEval += "          ";
        sFeedTotalModelEval += "           ";
        sFeedCacheHits += "           ";
        sFeedNbEval += "           ";
        sFeedSurrogateEval += "          ";
        sFeedNbRevealingIter += "           ";
    }
    if (showSurrogateEvalFromCacheForRerun)  // Longest title
    {
        sFeedBbEval += "                         ";
        sFeedBbEvalFromCacheForRerun += "";
        //sFeedLapBbEval += "";
        sFeedNbEvalNoCount += "      ";
        sFeedModelEval += "                       ";
        sFeedTotalModelEval += "                 ";
        sFeedCacheHits += "                              ";
        sFeedNbEval += "             ";
        sFeedSurrogateEval += "            ";
        sFeedSurrogateEvalFromCacheForRerun += "            ";
        sFeedNbRevealingIter += "                         ";
    }
    if (showLapBbEval)  // Longest title
    {
        sFeedBbEval += "                 ";
        //sFeedLapBbEval += "";
        sFeedNbEvalNoCount += "   ";
        sFeedModelEval += "                    ";
        sFeedTotalModelEval += "              ";
        sFeedCacheHits += "                           ";
        sFeedNbEval += "          ";
        sFeedSurrogateEval += "         ";
        sFeedNbRevealingIter += "                 ";
    }
    else if (showNbEvalNoCount) // Second longest
    {
        sFeedBbEval += "              ";
        //sFeedLapBbEval += "";
        //sFeedNbEvalNoCount += "";
        sFeedModelEval += "                 ";
        sFeedTotalModelEval += "           ";
        sFeedCacheHits += "                        ";
        sFeedNbEval += "       ";
        sFeedSurrogateEval += "         ";
        sFeedSurrogateEvalFromCacheForRerun += "         ";
        sFeedNbRevealingIter += "              ";
    }
    else if (showNbEval)    // 3rd longest title
    {
        sFeedBbEval += "        ";
        //sFeedLapBbEval += "";
        //sFeedNbEvalNoCount += "";
        sFeedModelEval += "   ";
        sFeedTotalModelEval += "     ";
        sFeedCacheHits += "                  ";
        sFeedNbEval += " ";
        sFeedSurrogateEval += "";
        sFeedNbRevealingIter  += "        ";
    }
    else if (showTotalModelEval)
    {
        sFeedBbEval += "   ";
        //sFeedLapBbEval += "";
        //sFeedNbEvalNoCount += "";
        sFeedModelEval += " ";
        //sFeedTotalModelEval += "    ";
        //sFeedCacheHits += "                 ";
        //sFeedNbEval += "";
        sFeedSurrogateEval += "         ";
        sFeedSurrogateEvalFromCacheForRerun += "         ";
    }


    size_t surrogateCost = 0;
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    if (nullptr != evc)
    {
        surrogateCost = evc->getEvaluatorControlGlobalParams()->getAttributeValue<size_t>("EVAL_SURROGATE_COST");
    }
    
    std::string sBbEval           = "Blackbox evaluations: " + sFeedBbEval + NOMAD::itos(bbEval);
    std::string sBbEvalFromCacheForRerun = "Blackbox evaluations from cache (rerun): " + sFeedBbEvalFromCacheForRerun + NOMAD::itos(bbEvalFromCacheForRerun);
    std::string sLapBbEval        = "Sub-optimization blackbox evaluations: " + sFeedLapBbEval + NOMAD::itos(lapBbEval);
    std::string sNbEvalNoCount    = "Blackbox evaluation (not counting): " + sFeedNbEvalNoCount + NOMAD::itos(nbEvalNoCount);
    std::string sModelEval        = "Model evaluations: " + sFeedModelEval + NOMAD::itos(modelEval);
    std::string sTotalModelEval   = "Total model evaluations: " + sFeedTotalModelEval + NOMAD::itos(totalModelEval);
    std::string sCacheHits        = "Cache hits: " + sFeedCacheHits + NOMAD::itos(nbCacheHits);
    std::string sNbEval           = "Total number of evaluations: " + sFeedNbEval + NOMAD::itos(nbEval);
    std::string sSurrogateEval    = "Static surrogate evaluations: " + sFeedSurrogateEval + NOMAD::itos(surrogateEval) ;
    std::string sNbRevealingIter    = "Revealing iterations: " + sFeedNbRevealingIter + NOMAD::itos(nbRevealingIter) ;
    if (surrogateCost > 0)
    {
        sSurrogateEval    += " -> Counts for " + NOMAD::itos(size_t(surrogateEval/surrogateCost)) + " blackbox evals.";
    }
    std::string sSurrogateEvalFromCacheForRerun    = "Static surrogate evaluations (cache rerun): " + sFeedSurrogateEvalFromCacheForRerun + NOMAD::itos(surrogateEvalFromCacheForRerun);
    std::string sLapSurrogateEval = "Sub-optimization static surrogate evaluations: " + sFeedLapSurrogateEval + NOMAD::itos(lapSurrogateEval);

#ifdef TIME_STATS
    std::string sTotalRealTime  = "Total real time (round s):    " + std::to_string(_totalRealAlgoTime);
    std::string sTotalCPUTime   = "Total CPU time (s):           " + std::to_string(_totalCPUAlgoTime);
#endif // TIME_STATS

    AddOutputInfo("", outputLevelHigh); // skip line
    // Always show number of blackbox evaluations
    AddOutputInfo(sBbEval, outputLevelHigh);
    // The other values are conditional to the show* booleans
    if (showbbEvalFromCacheForRerun)
    {
        AddOutputInfo(sBbEvalFromCacheForRerun, outputLevelNormal);
    }
    if (showLapBbEval)
    {
        AddOutputInfo(sLapBbEval, outputLevelNormal);
    }
    if (showNbEvalNoCount)
    {
        AddOutputInfo(sNbEvalNoCount, outputLevelNormal);
    }
    if (showSurrogateEval)
    {
        AddOutputInfo(sSurrogateEval, outputLevelNormal);
    }
    if (showSurrogateEvalFromCacheForRerun)
    {
        AddOutputInfo(sSurrogateEvalFromCacheForRerun, outputLevelNormal);
    }
    if (showModelEval)
    {
        AddOutputInfo(sModelEval, outputLevelNormal);
    }
    if (showTotalModelEval)
    {
        AddOutputInfo(sTotalModelEval, outputLevelNormal);
    }
    if (showNbCacheHits)
    {
        AddOutputInfo(sCacheHits, outputLevelNormal);
    }
    if (showNbEval)
    {
        AddOutputInfo(sNbEval, outputLevelNormal);
    }
    if (showNbRevealingIter)
    {
        AddOutputInfo(sNbRevealingIter, outputLevelNormal);
    }

#ifdef TIME_STATS
    {
        AddOutputInfo(sTotalRealTime, outputLevelNormal);
        AddOutputInfo(sTotalCPUTime, outputLevelNormal);
    }
#endif // TIME_STATS
}

NOMAD::EvalPoint NOMAD::Algorithm::getBestSolution(bool bestFeas) const
{
    NOMAD::EvalPoint bestSol;

    auto fixedVariable = NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this);

    auto barrier = getMegaIterationBarrier();
    if (nullptr != barrier)
    {
        NOMAD::EvalPointPtr bestSolPtr = nullptr;
        if (bestFeas)
        {
            bestSolPtr = barrier->getCurrentIncumbentFeas();
        }
        else
        {
            bestSolPtr = barrier->getCurrentIncumbentInf();
        }
        if (nullptr != bestSolPtr)
        {
            bestSol = bestSolPtr->makeFullSpacePointFromFixed(fixedVariable);
        }
    }
    
    return bestSol;
}

bool NOMAD::Algorithm::terminate(const size_t iteration)
{
    return _termination->terminate(iteration);
}


void NOMAD::Algorithm::display ( std::ostream& os ) const
{

    os << "MEGA_ITERATION " << std::endl;
    os << *_refMegaIteration << std::endl;
    os << "NB_EVAL " << NOMAD::EvcInterface::getEvaluatorControl()->getNbEval() << std::endl;
    os << "NB_BB_EVAL " << NOMAD::EvcInterface::getEvaluatorControl()->getBbEval() << std::endl;
    uint32_t x, y, z;
    NOMAD::RNG::getPrivateSeed(x, y, z);
    os << "RNG " << x << " " << y << " " << z << std::endl;

}


std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::Algorithm & mads)
{
    mads.display(os);
    return os;
}


void NOMAD::Algorithm::read(std::istream& is)
{
    // Read line by line
    std::string name;

    int nbEval = 0, nbBbEval = 0;
    uint32_t x, y, z;

    while (is >> name && is.good() && !is.eof())
    {
        if ("MEGA_ITERATION" == name)
        {
            is >> *_refMegaIteration;
        }
        else if ("NB_EVAL" == name)
        {
            is >> nbEval;
        }
        else if ("NB_BB_EVAL" == name)
        {
            is >> nbBbEval;
        }
        else if ("RNG" == name)
        {
            is >> x >> y >> z;
            NOMAD::RNG::setPrivateSeed(x, y, z);
        }
        else
        {
            // Put back name to istream. Maybe there is a simpler way.
            for (unsigned i = 0; i < name.size(); i++)
            {
                is.unget();
            }
            break;
        }
    }

    NOMAD::EvcInterface::getEvaluatorControl()->setBbEval(nbBbEval);
    NOMAD::EvcInterface::getEvaluatorControl()->setNbEval(nbEval);

}

size_t NOMAD::Algorithm::getNbObj()
{
    return NOMAD::getNbObj(getBbOutputType());
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::Algorithm& algo)
{
    algo.read(is);
    return is;

}
