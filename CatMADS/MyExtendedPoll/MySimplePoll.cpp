
#include "Algos/AlgoStopReasons.hpp"
#include "Algos/Mads/DoublePollMethod.hpp"
#include "Algos/SubproblemManager.hpp"
#include "Cache/CacheBase.hpp"
#include "Output/OutputQueue.hpp"
#include "Type/DirectionType.hpp"

#include "MySimplePoll.hpp"


/// <#Description#>
void MySimplePoll::init()
{
    setStepType(NOMAD::StepType::SIMPLE_POLL);
    
    // Rho parameter of the progressive barrier. Used to choose if the primary frame center is the feasible or infeasible incumbent.
    _rho = _runParams->getAttributeValue<NOMAD::Double>("RHO");

    // Get fixed variables from MySimpleMads
    _fixedVariable = NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(_parentStep);
    
    _n = _pbParams->getAttributeValue<size_t>("DIMENSION");
    _nSimple = _fixedVariable.size();
    _nbOutputs = _bbot.size();
    
    // Evaluated X0
    const auto X0s = _pbParams->getAttributeValue<NOMAD::ArrayOfPoint>("X0");
    // TODO: check why X0s=(0,0,0,0)
    //cout << X0s << endl;

    if (X0s.size() != 1 && !X0s[0].isComplete())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"My Simple Mads needs a single valid X0.");
    }
    
    NOMAD::SimpleEvalPoint evalPointX0(X0s[0]);

    _trialPoints.push_back(evalPointX0);
    evalTrialPoints(); // Compute f and h according to standard method
    
    // Take _trialPoints containing only X0s that is evaluated
    _extendedPollFrameCenter = _trialPoints[0];


    // Create a barrier from X0. Two cases
    // 1- PhaseOne (X0 has EB constraints not feasible -> h(X0)=INF) -> first step: minizes infeasibility
    // 2- Regular (h(X0) != INF)
    if ( NOMAD::INF == _trialPoints[0].getH() )
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"My Simple Mads cannot handle h(X0)=INF.");
//        _phaseOneSearch = true;
//
//        // Recompute f and h for trial points using PhaseOne
//        _trialPoints.clear();
//        _trialPoints.push_back(evalPointX0);
//        evalTrialPoints();
        
    }
    _barrier = std::make_unique<NOMAD::SimpleProgressiveBarrier>(NOMAD::INF,
                                                                 NOMAD::Point(_n) /*undefined fixed variables */,
                                                                 _trialPoints);
    
    // Make x0 the ref bests
    _barrier->updateRefBests();
    
    // Create
    _mesh = std::make_shared<NOMAD::GMesh>(_pbParams, _runParams);
    
    // No need to enforce sanity check
    _mesh->setEnforceSanityChecks(false);

}


bool MySimplePoll::runImp()
{
    // Clear points from previous iterations
    _trialPoints.clear();

    // 1- Create poll methods and generate points (just one pass)
    generateTrialPoints();
    
    if ( _trialPoints.size() > 0 )
    {

        // 2- Evaluate points
        _myPollSuccessType = MySuccessType::UNDEFINED;
        evalTrialPoints();

        // In evalTrialPoints() we opportunistically stop evaluating if better solutions is found
        //if (!(_myPollSuccessType == MySuccessType::MEGA_SUCCESS))
        //{
           // Only update the barrier if a better solution is found 
           // TODO: check if we want to update the barrier;
           // 3- Update barrier
        //   _barrier->updateWithPoints(_trialPoints);
        //}

        return true;
    }
    else
    {
        return false;
    }

    
}


void MySimplePoll::endImp()
{
    // V1 of the Extended Poll does not modify the mesh size and the barrier
    // Update frame center based on the trial points
    
    // Skip this step if MEGA_SUCCESS has been found in the preceeding evaluation step
    if (!(_myPollSuccessType == MySuccessType::MEGA_SUCCESS))
    {
        // Determine if the iteration is FULL_SUCCESS, i.e. a dominating pt is found, but it's not better than incumbents
        bool isFullSuccess = false;

        for (size_t i = 0 ; i < _trialPoints.size() ; i++){
            
            // Check if there's a trialPoint dominating the current frame center
            // If there such pt, then update the frame center and set FULL_SUCCESS to continue the extended poll
            cout << _trialPoints[i].getF() << ", " << _trialPoints[i].getH() << " vs " << _extendedPollFrameCenter.getF() << ", " << _extendedPollFrameCenter.getH() << endl;
            if (dominates(_trialPoints[i], _extendedPollFrameCenter, _hMaxMainAlgo))
            {
                //cout << "At full success, best feas:" <<_refBestFeas.display() << endl;
                //cout << "At full success, best inf:" << _refBestInf.display() << endl;
                isFullSuccess = true;
                _extendedPollFrameCenter = _trialPoints[i];
                cout << "New Extended Poll Frame Center:" << _extendedPollFrameCenter.display() << " with " << _extendedPollFrameCenter.getF() << endl;
                _myPollSuccessType = MySuccessType::FULL_SUCCESS;

                // TODO later increase frame and mesh size 
                break;
            }
        }

        if (isFullSuccess == false){
            _myPollSuccessType == MySuccessType::UNSUCCESSFUL;
        }

    }

    // Stop at start next iteration
    _trialPoints.clear();

}


// BestRef points are not SimpleEvalPoint but rather EvalPoint
bool MySimplePoll::dominatesRef(const NOMAD::SimpleEvalPoint & p1, const NOMAD::EvalPoint & bestRef, const NOMAD::Double hMax) const
{
    NOMAD::FHComputeType CT = NOMAD::defaultFHComputeType;

    if ( !p1.isDefined() || !(bestRef.getEvalStatus(NOMAD::EvalType::BB) == NOMAD::EvalStatusType::EVAL_OK))
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"F1, H1, F2 and H2 must be defined.");
    }
    
    if (p1.getH() <=0 && bestRef.getH(CT)<=0) // Both feasible
    {
        // 1 and 2 are both feasible, and 1 dominates 2.
        if (p1.getF() < bestRef.getF(CT))
        {
            return true;
        }
    }
    else if (p1.getH() > 0 && bestRef.getH(CT) > 0) // Both infeasible
    {
        if (p1.getH() != NOMAD::INF && p1.getH() <= hMax)
        {
            if ((p1.getF() <= bestRef.getF(CT)) && (p1.getH() <= bestRef.getH(CT)) && ((p1.getF() < bestRef.getF(CT)) || (p1.getH() < bestRef.getH(CT))))
            {
                return true;
            }
        }
    }

    // ----------- NEW ------------------- //
    // Only one amongst p1 and p2 is feasible
    else
    {
        // p1 is feasible and p2 is unfeasible
        if (p1.getH()<=0)
        {
            // p1 is feasible and has better objective value
            if (p1.getF() <= bestRef.getF(CT))
            {
                return true;
            }
            // p1 does not have better objective value
            else{
                return false;
            }

        }
        // p1 is unfeasible and p2 is feasible => p1 can't dominate p2, because it has worst h value
        else{
            return false;
        }
    }
    // ----------- NEW ------------------- //


    return false;
}


bool MySimplePoll::dominates(const NOMAD::SimpleEvalPoint & p1, const NOMAD::SimpleEvalPoint & p2, const NOMAD::Double hMax) const
{
    // Comparing objective vectors of different size is undefined
    if ( !p1.isDefined() || !p2.isDefined())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"F1, H1, F2 and H2 must be defined.");
    }
    
    if (p1.getH() <=0 && p2.getH() <=0) // Both feasible
    {
        // 1 and 2 are both feasible, and 1 dominates 2.
        if (p1.getF() < p2.getF())
        {
            return true;
        }
    }
    else if (p1.getH() > 0 && p2.getH() > 0) // Both infeasible
    {
        if (p1.getH() != NOMAD::INF && p1.getH() <= hMax)
        {
            if ((p1.getF() <= p2.getF()) && (p1.getH() <= p2.getH()) && ((p1.getF() < p2.getF()) || (p1.getH() < p2.getH())))
            {
                return true;
            }
        }
    }

    // ----------- NEW ------------------- //
    // Only one amongst p1 and p2 is feasible
    else
    {
        // p1 is feasible and p2 is unfeasible
        if (p1.getH()<=0)
        {
            // p1 is feasible and has better objective value
            if (p1.getF() <= p2.getF())
            {
                return true;
            }
            // p1 does not have better objective value
            else{
                return false;
            }

        }
        // p1 is unfeasible and p2 is feasible => p1 can't dominate p2, because it has worst h value
        else{
            return false;
        }
    }
    // ----------- NEW ------------------- //

    return false;
}



// Generate new points to evaluate
void MySimplePoll::generateTrialPoints()
{

    // Create poll method from frame center
    std::shared_ptr<NOMAD::PollMethodBase> pollMethod;
    auto fc = std::make_shared<NOMAD::EvalPoint>(_extendedPollFrameCenter);
    pollMethod = std::make_shared<NOMAD::DoublePollMethod>(this, fc);

    // Generate trial points. Snap to bounds and project on mesh is also done.
    pollMethod->generateTrialPoints();

    // Pass the points from Poll method to Poll for later evaluation
    const auto & pollMethodPoints = pollMethod->getTrialPoints();
    for (const auto & point : pollMethodPoints)
    {
        _trialPoints.push_back(static_cast<NOMAD::SimpleEvalPoint>(point));
    }
    // ------------ //
    // Important note: the frame center must be updated after the trial points are evaluated
    // ------------ //


    // Old
    //createPollMethodsForPollCenters();
    // Use poll methods to create trial points
    //for (auto pollMethod : _pollMethods)
    //{
        // Generate trial points. Snap to bounds and project on mesh is also done.
    //    pollMethod->generateTrialPoints();

        // Pass the points from Poll method to Poll for later evaluation
    //    const auto & pollMethodPoints = pollMethod->getTrialPoints();
    //    for (const auto & point : pollMethodPoints)
    //    {
    //        _trialPoints.push_back(static_cast<NOMAD::SimpleEvalPoint>(point));
    //    }
    //}
}


void MySimplePoll::computePrimarySecondaryPollCenters(NOMAD::SimpleEvalPoint & primaryCenter,
    NOMAD::SimpleEvalPoint  &secondaryCenter) const
{

auto firstXIncFeas = _barrier->getCurrentIncumbentFeas();
auto firstXIncInf  = _barrier->getCurrentIncumbentInf();
bool primaryIsInf = false;

// Negative rho means make no distinction between primary and secondary polls.
const bool usePrimarySecondary = (_rho >= 0) && (firstXIncFeas.isDefined()) && (firstXIncInf.isDefined());
if (usePrimarySecondary)
{

    const NOMAD::Double & fFeas = firstXIncFeas.getF();
    const NOMAD::Double & fInf  = firstXIncFeas.getF();

    if (fFeas.isDefined() && fInf.isDefined())
    {
        // Single objective case
        if ((fFeas - _rho) > fInf)
        {
            // xFeas' f is too large, use xInf as primary poll instead.
            primaryIsInf = true;
        }
        // TODO multiobjective. See how it is done in regular Poll

    }
    }

    if (usePrimarySecondary)
    {
        if (primaryIsInf)
        {
            primaryCenter=firstXIncInf;
            secondaryCenter=firstXIncFeas;
        }
    else
    {
        primaryCenter=firstXIncFeas;
        secondaryCenter=firstXIncInf;
    }
    }
    else
    {
    if (firstXIncFeas.isDefined())
        {
        primaryCenter=firstXIncFeas;
        }
    else
    {
        if (firstXIncInf.isDefined())
        {
        primaryCenter=firstXIncInf;
        }
    }
    }
}


void MySimplePoll::createPollMethod(const bool isPrimary, const NOMAD::SimpleEvalPoint & frameCenter)
{

    if ( !frameCenter.isDefined())
    {
        return;
    }
    
    // Select the poll methods to be executed
    std::shared_ptr<NOMAD::PollMethodBase> pollMethod;
    auto fc = std::make_shared<NOMAD::EvalPoint>(frameCenter);

    
    // TODO EHH
    if (isPrimary)
    {
        // pollMethod = std::make_shared<NOMAD::Ortho2NPollMethod>(this, fc);
        pollMethod = std::make_shared<NOMAD::DoublePollMethod>(this, fc);
    }
    else
    {
        pollMethod = std::make_shared<NOMAD::DoublePollMethod>(this, fc);
    }
    _frameCenters.push_back(frameCenter);
    _pollMethods.push_back(pollMethod);

}


void MySimplePoll::createPollMethodsForPollCenters()
{
    // Compute primary and secondary poll centers
    NOMAD::SimpleEvalPoint primaryCenter, secondaryCenter;
    computePrimarySecondaryPollCenters(primaryCenter, secondaryCenter);

    // Add poll methods for primary polls
    _pollMethods.clear();
    _frameCenters.clear();

    createPollMethod(true, primaryCenter);

    // Add poll methods for secondary polls
    createPollMethod(false, secondaryCenter);
}


void MySimplePoll::evalTrialPoints()
{
    // TODO: Can we use EvaluatorControl and use a custom opportunistic stop?
    // Alternative approach to check in the future 
    
    bool countEval = true;
    NOMAD::Double hMax;
    NOMAD::FHComputeType computeType = NOMAD::defaultFHComputeType;
    for (size_t i = 0 ; i < _trialPoints.size() ; i++)
    {
        // Convert SimpleEvalPoint into EvalPoint
        // Account for fixed variable
        NOMAD::EvalPoint ep(_nSimple);
        size_t k = 0;
        for (size_t j = 0; j < _nSimple; j++)
        {
            if ( _fixedVariable[j].isDefined() )
            {
                ep.set(j,_fixedVariable[j].todouble());
            }
            else
            {
                ep.set(j,_trialPoints[i][k]);
                k++;
            }
        }
        
        // Smart insert (not done if already in cache) trial point in cache.
        // Tag must be set before insert.
        auto evalType = _evaluator->getEvalType();
        ep.updateTag();
        bool doEval = NOMAD::CacheBase::getInstance()->smartInsert(ep, 1, evalType);
        
        if (doEval)
        {
            // Complete EvalPoint information needed for eval and for cache
            ep.setEvalStatus(NOMAD::EvalStatusType::EVAL_IN_PROGRESS, evalType );
            
            // Perform evaluation
            bool evalOk = _evaluator->eval_x(ep, hMax, countEval);
            
            if (countEval)
            {
                _nbEval++;
                
                // Update nb eval in evaluator control.
                auto evc = NOMAD::EvcInterface::getEvaluatorControl();
                auto currentBBEval = evc->getBbEval();
                evc->setBbEval(currentBBEval + 1);
            }
            
            
            if (evalOk)
            {
                // Set bb output type list to be able to compute f and h later.
                ep.getEval(evalType)->setBBOutputTypeList(_bbot);
                
                // Update status of trial point in cache
                ep.setEvalStatus(NOMAD::EvalStatusType::EVAL_OK, evalType );
                
                // Set F and H of SimpleEvalPoint
                _trialPoints[i].setF(ep.getF(computeType));
                _trialPoints[i].setH(ep.getH(computeType));
                
                // Keep track of the NOMAD::EvalPoint (for passing to the ExtendedPoll parent barrier)
                _allEvaluatedTrialPoints.push_back(ep);
                
                
                // If the first frame center is feasible, then compare with best feasible
                // Otherwise, compare with the best infeasible
                if ((_firstFrameCenter.isFeasible(computeType) && dominatesRef(_trialPoints[i], *_refBestFeas, _hMaxMainAlgo)) ||
                            (!_firstFrameCenter.isFeasible(computeType) && dominatesRef(_trialPoints[i], *_refBestInf, _hMaxMainAlgo) && _trialPoints[i].getH()<_hMaxMainAlgo))
                {
                    if (NOMAD::OutputQueue::GoodLevel(NOMAD::OutputLevel::LEVEL_INFO))
                    {
                        AddOutputInfo("Trial point dominates frame center! ");
                    }

                    if (_refBestFeas != nullptr){
                        cout << "At Mega success, best feas:" << (*_refBestFeas).display() << endl;
                    }
                    if (_refBestInf != nullptr){
                        cout << "At Mega success, best inf:" << (*_refBestInf).display() << endl;
                    }

                    // Set success at MEGA_SUCCESS since we have found a better global solution and opportunistically break
                    _myPollSuccessType = MySuccessType::MEGA_SUCCESS;
           
                }
            }
            else
            {
                ep.setEvalStatus(NOMAD::EvalStatusType::EVAL_FAILED, evalType );
            }
            
            // Update cache trial points
            NOMAD::CacheBase::getInstance()->update(ep, evalType);
            
            // Some display
            if (NOMAD::OutputQueue::GoodLevel(NOMAD::OutputLevel::LEVEL_INFO))
            {
                if (!evalOk)
                {
                    AddOutputInfo("MyExtendedPoll: Eval not ok for one of the trial point ");
                }
                if (!countEval)
                {
                    AddOutputInfo("MyExtendedPoll: Not counting this eval! ");
                }
            }
            
            if (NOMAD::OutputQueue::GoodLevel(NOMAD::OutputLevel::LEVEL_NORMAL))
            {
                //cout << "Testing, best feas:" <<_refBestFeas.display() << endl;
                //cout << "Testing, best inf:" << _refBestInf.display() << endl;
                
                std::cout << "MyExtendedPoll eval trial point: " << ep.getX()->display() << ") f= " << ep.getF(computeType) << " h= "  << ep.getH(computeType) <<std::endl;
            }
            
            writeStats(ep);
            
        }
        else
        {
            // EvalPoint is in cache, let's use it
            // Update ep from cache to obtain bbo values
            NOMAD::EvalPoint epFound;
            NOMAD::CacheBase::getInstance()->find(*ep.getX(), epFound);
            // Set F and H of SimpleEvalPoint
            _trialPoints[i].setF(epFound.getF(computeType));
            _trialPoints[i].setH(epFound.getH(computeType));
            
        }

        // At the end, break the loop is MEGA_SUCCESS is found
        // We do it at the end to output the ExtendedPoint
        if (_myPollSuccessType == MySuccessType::MEGA_SUCCESS){
            break;
            // Sanity clear on the trial points with early stopping
            _trialPoints.clear();
        }


    }

}

void MySimplePoll::writeStats(const NOMAD::EvalPoint & ep) const
{
    // Evaluation info for output
    NOMAD::StatsInfoUPtr stats(new NOMAD::StatsInfo());
    
    auto ec = NOMAD::EvcInterface::getEvaluatorControl();
    
    NOMAD::FHComputeType computeType;
    
    stats->setFailEval(!ep.isEvalOk(computeType.evalType));
    stats->setObj(ep.getF(computeType));
    stats->setConsH(ep.getH(computeType));
    stats->setSol(*(ep.getX()));
    stats->setBBE(ec->getBbEval());
    stats->setBBO(ep.getBBO(computeType.evalType));
    stats->setTag(ep.getTag());
    NOMAD::OutputInfo outputInfo("MySimplePoll", "", NOMAD::OutputLevel::LEVEL_STATS);
    outputInfo.setStatsInfo(std::move(stats));
    NOMAD::OutputQueue::Add(std::move(outputInfo));
    
    NOMAD::OutputQueue::Flush();
    
    
}
