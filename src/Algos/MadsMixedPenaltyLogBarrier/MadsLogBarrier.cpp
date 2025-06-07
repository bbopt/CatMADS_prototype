
#include "../../Algos/CacheInterface.hpp"
#include "../../Algos/MainStep.hpp"
#include "../../Algos/MadsMixedPenaltyLogBarrier/MadsLogBarrier.hpp"
#include "../../Algos/Mads/MadsInitialization.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/Mads/MadsUpdate.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Eval/ProgressiveBarrier.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Util/fileutils.hpp"
#ifdef TIME_STATS
#include "../../Util/Clock.hpp"
#endif

void NOMAD::MadsLogBarrier::init()
{
    // Define the single-objective merit function with inequality constraints as penalty.
    std::function<NOMAD::Double(const NOMAD::BBOutputTypeList& bbOutputTypeList,
                                const NOMAD::BBOutput& bbOutput)> meritZCompute = [&](const NOMAD::BBOutputTypeList& bbOutputTypeList,
                                                                                      const NOMAD::BBOutput& bbOutput) -> NOMAD::Double
    {
        if (!bbOutput.getEvalOk() || bbOutputTypeList.empty())
        {
            return NOMAD::INF;
        }

        if (!bbOutput.checkSizeMatch(bbOutputTypeList))
        {
            return NOMAD::INF;
        }

        NOMAD::Double meritZ = 0;
        const NOMAD::ArrayOfDouble allC = bbOutput.getConstraints(bbOutputTypeList);

        if (allC.size() == 0)
        {
            // This is in case the algo is used for an unconstrained pb!
            meritZ += bbOutput.getObjective(bbOutputTypeList);
            return meritZ;
        }

        // Initialization of Glog and Gext (not for _exteriorPenaltyOnly)
        if (!_exteriorPenaltyOnly && _Glog.empty() && _Gext.empty())
        {
            // Initialization of indices for Glog and Gext (done once)
            for (size_t i=0 ; i < allC.size(); i++)
            {
                if (allC[i].todouble() < 0)
                {
                    _Glog.push_back(i);
                }
                else
                {
                    _Gext.push_back(i);
                }
            }
        }

        // Initialization of rhoExt
        if (!_rhoExt.isDefined())
        {
            NOMAD::ArrayOfDouble Fs = bbOutput.getObjectives(bbOutputTypeList);
            if (Fs.size() > 1)
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"Cannot handle multi-objective in MadsLogBarrier");
            }
            _rhoExt = 1.0 / max(Fs[0].abs(),10.0);

            // TMP for testing
            // Testing 1: 0.01, Testing 2: 0.001
            _rhoExt *= 0.001;
        }
        // Compute merit function: two situations, exterior only OR log barrier and exterior
        if (_exteriorPenaltyOnly)
        {
            meritZ = 0;
            for (size_t i=0 ; i < allC.size(); i++)
            {
                if (allC[i].todouble() > 0)
                {
                    meritZ += pow(allC[i].todouble(), _nu)/_rhoExt;
                }
            }
        }
        else
        {
            double maxInfeas = -INF;
            for (const auto i: _Glog)
            {

                // Relaxed feasibility wrt threshold
                if (allC[i].todouble() < _logFeasThres.todouble())
                {
                    if (_logVariant)
                    {
                        maxInfeas = std::max(maxInfeas,allC[i].todouble()-_logFeasThres.todouble());
                    }
                    else
                    {
                        meritZ -= log(-std::max(-1.0,allC[i].todouble()-_logFeasThres.todouble()));
                    }
                }
                else
                {
                    meritZ = NOMAD::INF;
                    return meritZ;
                }
            }
            if (_logVariant)
            {
                meritZ = -log(-std::max(-1.0,maxInfeas-_logFeasThres.todouble()));
            }
            meritZ *=_rhoLog;
            for (const auto i: _Gext)
            {
                // Enforce "infeasibility" wrt threshold
                if (allC[i].todouble() > _logFeasThres.todouble())
                {
                    meritZ += pow(allC[i].todouble(), _nu)/_rhoExt;
                }
            }
        }

        meritZ += bbOutput.getObjective(bbOutputTypeList);

        return meritZ;
    };

    // The mads log barrier algorithm works with exterior penalty and interior log barrier penalty.
    // The mads exterior penalty algorithm works only with exterior penalty and no log barrier penalty.
    _exteriorPenaltyOnly = _runParams->getAttributeValue<bool>("MADSEXTERIORPENALTY_OPTIMIZATION");

    // Temp for testings.
    // A variant of the log barrier merit compute.
    _logVariant = _runParams->getAttributeValue<bool>("MADSLOGBARRIER_OPTIMIZATION_LOGVARIANT");
    // Log to exterior switch
    _logToExtSwitch = _runParams->getAttributeValue<bool>("MADSLOGBARRIER_OPTIMIZATION_LOGTOEXTSWITCH");
    // Feasibility threshold for log barrier
    _logFeasThres = _runParams->getAttributeValue<NOMAD::Double>("MADSLOGBARRIER_OPTIMIZATION_FEASIBILITY_THRESHOLD");
    if (_logFeasThres < 0)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"MADSLOGBARRIER_OPTIMIZATION_FEASIBILITY_THRESHOLD must be positive");
    }

    // Make the penalized merit function compute available to the evaluator control.
    // The infeasibility H function must always return 0 (feasible)
    // Must be placed after main step start
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    evc->setComputeType(NOMAD::ComputeType::USER, meritZCompute, [](const NOMAD::BBOutputTypeList& bbOutputTypeList,
                                                                    const NOMAD::BBOutput& bbOutput){return NOMAD::Double(0);});

}


void NOMAD::MadsLogBarrier::startImp()
{
    // Default start to perform Mads initialization (evalX0,....)
    NOMAD::Mads::startImp();

    /*-----------------------------------------------------*/
    /* Mega Iteration start callback                       */
    /* Update objective penalization weights               */
    /*-----------------------------------------------------*/
    auto userIterationCallback = [&](const NOMAD::Step& step,
                                     bool &stop) -> void
    {
        // Important: by default USER_CALLS are disabled when doing quad model optimization.

        // Several NOMAD::Algorithm are used by NOMAD.
        // We are interested only on the main Mads (Mega) Iteration.
        // Use a dynamic cast to make sure with have the Mads (Mega) Iteration.
        auto iter = dynamic_cast<const NOMAD::MadsMegaIteration*>(&step);

        if (nullptr != iter)
        {
            const auto & barrier = iter->getBarrier();
            const auto & bf = barrier->getCurrentIncumbentFeas();
            const auto & evalType = barrier->getEvalType();
            const auto & computeType = barrier->getFHComputeType().Short() ;

            bool rhoLogHasChanged = false, rhoExtHasChanged = false, typeHasChanged = false;

            NOMAD::ArrayOfDouble allG = bf->getEval(evalType)->getBBOutputByType(NOMAD::BBOutputType::PB);

            // ChT Temp for debugging. TODO use OutputQueue
            // std::cout << "Best feasible before update: PB (" << allG << "), OBJ ( "<< bf->getEval(evalType)->getBBOutputByType(NOMAD::BBOutputType::OBJ) << " ), meritZ = " << bf->getEval(evalType)->getFs(computeType).display()  <<std::endl;

            if (_exteriorPenaltyOnly)
            {
                const double comp = _cRhoExt*pow(_rhoExt.todouble(),_beta) ;
                const auto & maxFrameSize = iter->getMesh()->getDeltaFrameSize().max();
                if ( maxFrameSize.todouble() < comp )
                {
                    _rhoExt *= _zeta;
                    rhoExtHasChanged = true;
                }
            }
            else
            {
                NOMAD::Double gmin = NOMAD::INF;

                for (const auto i: _Glog)
                {
                    double g = std::fabs(allG[i].todouble());
                    if (g < gmin.todouble())
                    {
                        gmin = g;
                    }
                }
                double comp;
                if (gmin < NOMAD::INF)
                {
                    comp = std::min( pow(_rhoLog.todouble(),_beta), _cRhoLog*pow(gmin.todouble(),2.0) );
                }
                else
                {
                    comp = pow(_rhoLog.todouble(),_beta);
                }

                const auto & maxFrameSize = iter->getMesh()->getDeltaFrameSize().max();
                if ( maxFrameSize.todouble() < comp )
                {
                    _rhoLog *= _zeta;
                    rhoLogHasChanged = true;

                    comp = _cRhoExt*pow(_rhoExt.todouble(),_beta) ;
                    if ( maxFrameSize.todouble() < comp )
                    {
                        _rhoExt *= _zeta;
                        rhoExtHasChanged = true;
                    }
                }

                // Implement switch of type Gext -> Glog
                auto it=_Gext.begin();
                while (it != _Gext.end())
                {
                    // We can switch from _Gext to _Glog if constraints becomes feasible
                    // And if it has not already been switched from _Glog to _Gext
                    if (allG[*it].todouble() <= -1E-14 && _GSwitchedLogToExt.end() == std::find(_GSwitchedLogToExt.begin(), _GSwitchedLogToExt.end(), *it)  )
                    {

//                        // ChT Temp testing Switch back.
//                        if ( _GSwitchedLogToExt.end() != std::find(_GSwitchedLogToExt.begin(), _GSwitchedLogToExt.end(), *it))
//                        {
//                            if ( allG[*it].todouble() <= -1.0 )
//                            {
//                                std::cout<<"Switch back to log"<<std::endl;
//                                auto itS = std::find(_GSwitchedLogToExt.begin(), _GSwitchedLogToExt.end(), *it);
//                                _GSwitchedLogToExt.erase(itS);
//                            }
//                            else
//                            {
//                                it++;
//                                continue;
//                            }
//
//                        }

                        _Glog.push_back(*it);
                        it = _Gext.erase(it);
                        typeHasChanged = true;
                    }
                    else
                    {
                        it++;
                    }
                }

                if (_logToExtSwitch && !_Glog.empty())
                {
                    // Implement switch of type Glog -> Gext

                    // Proximity of Omega_log boundary
                    // The max of allG for elements in _Glog
                    auto maxIdxIt = (std::max_element(_Glog.begin(), _Glog.end(), [&](size_t idx1, size_t idx2)
                                              { return allG[idx1].todouble() < allG[idx2].todouble(); }
                                              ));
                    double proximity = allG[*maxIdxIt].todouble();
                    if (proximity>0)
                    {
                        throw NOMAD::Exception(__FILE__,__LINE__,"An element in Glog is infeasible");
                    }
                    if (proximity>-1)
                    {
                        // std::cout << "Proximity>-1"<< std::endl;

                        // Compute the sum of _Gext constraints violation
                        double violation = std::accumulate(_Gext.begin(), _Gext.end(), 0.0, [&](double sum, size_t idx) { return sum + std::max(0.0,allG[idx].todouble()); });

                        if (violation/std::fabs(proximity) >= 10)
                        {

                            std::cout<<"Switching"<<std::endl;

                            // Switch from Glog to Gext the constraint in Glog with the highest violation (maxIdx)
                            _Gext.push_back(*maxIdxIt);

                            // Add the violation to the list of switched constraints
                            _GSwitchedLogToExt.push_back(*maxIdxIt);

                            // Remove the element in _Glog having the index maxIdx
                            _Glog.erase(maxIdxIt);

                            typeHasChanged = true;

                        }
                    }
                }
            }

            // Reset bf
            if ( rhoExtHasChanged || rhoLogHasChanged || typeHasChanged)
            {
                bf->resetFValues();
                auto evc = NOMAD::EvcInterface::getEvaluatorControl();
                evc->getBestIncumbent(-1)->resetFValues();

                // Update barrier with the point from the cache having the best merit
                std::function<bool(const EvalPoint&)> func = [&, evalType, computeType](const EvalPoint& evalPoint) -> bool
                {
                    if(evalPoint.isEvalOk(evalType) && evalPoint.getEval(evalType)->isFeasible(computeType) && evalPoint.getEval(evalType)->getFs(computeType)[0] < bf->getEval(evalType)->getFs(computeType)[0])
                    {
                        return true;
                    }
                    return false;
                };
                std::vector<NOMAD::EvalPoint> evalPointList;
                NOMAD::CacheInterface cacheInterface(this);
                cacheInterface.find(func, evalPointList);
                barrier->updateWithPoints(evalPointList,false, true /*update incumbent*/);


                // ChT Temp for debugging. TODO use OutputQueue
                // std::cout << "Best feasible after update: meritZ = " << barrier->getCurrentIncumbentFeas()->getEval(evalType)->getFs(computeType).display()  <<std::endl;
            }
        }

    };

    // Set main step callback
    // Mega iteration start is after barrier update and mesh update
    auto mainStepAncestorConst = _parentStep->getParentOfType<NOMAD::MainStep*>(false);
    auto mainStepAncestor = const_cast<NOMAD::MainStep*>(mainStepAncestorConst);
    mainStepAncestor->addCallback(NOMAD::CallbackType::MEGA_ITERATION_START, userIterationCallback);

}


//void NOMAD::Mads::hotRestartOnUserInterrupt()
//{
//    if (_stopReasons->checkTerminate())
//    {
//        return;
//    }
//#ifdef TIME_STATS
//    if (isRootAlgo())
//    {
//        _totalCPUAlgoTime += NOMAD::Clock::getCPUTime() - _startTime;
//    }
//#endif // TIME_STATS
//    hotRestartBeginHelper();
//
//    // Reset mesh because parameters have changed.
//    std::stringstream ss;
//    const NOMAD::Iteration* iteration = getParentOfType<NOMAD::Iteration*>();
//    if (nullptr != iteration)
//    {
//        auto mesh = getIterationMesh();
//        ss << *mesh;
//        // Reset pointer
//        mesh.reset();
//
//        mesh = std::make_shared<NOMAD::GMesh>(iteration->getPbParams(),iteration->getRunParams());
//        // Get old mesh values
//        ss >> *mesh;
//    }
//
//    hotRestartEndHelper();
//#ifdef TIME_STATS
//    if (isRootAlgo())
//    {
//        _startTime = NOMAD::Clock::getCPUTime();
//    }
//#endif // TIME_STATS
//}
//
//
//void NOMAD::Mads::readInformationForHotRestart()
//{
//    // Restart from where we were before.
//    // For this, we need to read some files.
//    // Note: Cache file is treated independently of hot restart file.
//
//    if (_runParams->getAttributeValue<bool>("HOT_RESTART_READ_FILES"))
//    {
//        // Verify the files exist and are readable.
//        std::string hotRestartFile = _runParams->getAttributeValue<std::string>("HOT_RESTART_FILE");
//        if (NOMAD::checkReadFile(hotRestartFile))
//        {
//            std::string s = "Read hot restart file " + hotRestartFile;
//            NOMAD::OutputQueue::Add(s, NOMAD::OutputLevel::LEVEL_NORMAL);
//
//            // Create a GMesh and an MadsMegaIteration with default values, to be filled
//            // by istream is.
//            // NOTE: Working in full dimension
//            auto barrier = std::make_shared<NOMAD::ProgressiveBarrier>(NOMAD::INF, NOMAD::Point(_pbParams->getAttributeValue<size_t>("DIMENSION")), NOMAD::EvalType::BB);
//
//            std::shared_ptr<NOMAD::MeshBase> mesh = std::make_shared<NOMAD::GMesh>(_pbParams,_runParams);
//
//            _refMegaIteration = std::make_shared<NOMAD::MadsMegaIteration>(this, 0, barrier, mesh, NOMAD::SuccessType::UNDEFINED);
//
//            // Here we use Algorithm::operator>>
//            NOMAD::read<NOMAD::Mads>(*this, hotRestartFile);
//        }
//    }
//}
