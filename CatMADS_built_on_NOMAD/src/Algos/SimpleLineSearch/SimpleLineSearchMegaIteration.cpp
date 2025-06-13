#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"
#include "../../Algos/SimpleLineSearch/SimpleLineSearchMegaIteration.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::SimpleLineSearchMegaIteration::init()
{
    setStepType(NOMAD::StepType::MEGA_ITERATION);
    
    // Get all parameters required by the method
    _baseFactor = _runParams->getAttributeValue<NOMAD::Double>("SPECULATIVE_SEARCH_BASE_FACTOR");
    
    // The pb params handle only variables (fixed variables are not considered)
    _lb = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND");
    _ub = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND");

}


void NOMAD::SimpleLineSearchMegaIteration::startImp()
{
    if ( ! _stopReasons->checkTerminate() )
    {
        // Default mega iteration start tasks
        // See issue #639
        NOMAD::MegaIteration::startImp();
    }
}

bool NOMAD::SimpleLineSearchMegaIteration::runImp()
{
    bool successful = false;
    std::string s;

    if ( _stopReasons->checkTerminate() )
    {
        OUTPUT_DEBUG_START
        s = getName() + ": stopReason = " + _stopReasons->getStopReasonAsString() ;
        AddOutputDebug(s);
        OUTPUT_DEBUG_END
        return false;
    }

    // Generate points starting from all points in the barrier.
    // If FRAME_CENTER_USE_CACHE is false (default), that is the same
    // as using the best feasible and best infeasible points.
    
    std::shared_ptr<DMultiMadsBarrier> dMultiMadsBarrier = std::dynamic_pointer_cast<DMultiMadsBarrier>(_barrier);
    
    std::vector<NOMAD::EvalPoint> frameCenters;
    
    if (nullptr == _barrier)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearchMethod: null barrier");
    }
    auto computeType = _barrier->getFHComputeType();
    
    // DMultiMadsBarrier may contain too many points. Use only the current incumbents.
    if (nullptr != dMultiMadsBarrier)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearchMethod: not implemented for DMultiMads");
    }
    else
    {
        frameCenters = _barrier->getAllPoints();
    }
    
    for (const auto & frameCenter : frameCenters)
    {
        bool canGenerate = true;
        // Test that the frame center has a valid generating direction
        auto pointFrom = frameCenter.getPointFrom(NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
        if (nullptr == pointFrom || *pointFrom == frameCenter)
        {
            canGenerate = false;
        }
        
        if (canGenerate)
        {
            // Note: Recomputing direction, instead of using frameCenter.getDirection(),
            // to ensure we work in subspace.
            auto dir = NOMAD::Point::vectorize(*pointFrom, frameCenter);
            
            OUTPUT_INFO_START
            AddOutputInfo("Frame center: " + frameCenter.display());
            AddOutputInfo("Direction before scaling: " + dir.display());
            OUTPUT_INFO_END
            
            auto diri = dir;
            for(size_t j = 0 ; j < dir.size(); j++)
            {
                diri[j] *= _baseFactor;
            }
            
            OUTPUT_INFO_START
            AddOutputInfo("Scaled direction : " + diri.display());
            OUTPUT_INFO_END
            
            // Generate
            auto evalPoint = NOMAD::EvalPoint(*(pointFrom->getX()) + diri);
            
            // Insert the point
            evalPoint.setPointFrom(std::make_shared<NOMAD::EvalPoint>(frameCenter), NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
            evalPoint.addGenStep(getStepType());
            
            if (snapPointToBoundsAndProjectOnMesh(evalPoint, _lb, _ub))
            {

                bool inserted = insertTrialPoint(evalPoint);
                
                OUTPUT_INFO_START
                s = "Speculative point:";
                s += (inserted) ? " inserted " : " not inserted: ";
                s += evalPoint.display();
                AddOutputInfo(s);
                OUTPUT_INFO_END
            }
            
            
            // Eval the trial point and update the barrier
            bool foundBetter = false;
            
            if ( ! _stopReasons->checkTerminate() )
            {
                foundBetter = evalTrialPoints(this);
            }
            // From IterationUtils. Update megaIteration barrier.
            postProcessing();
            clearTrialPoints();
            
            auto evc = NOMAD::EvcInterface::getEvaluatorControl();
            auto evalType = evc->getCurrentEvalType();
            
            // Perform a simple line search if
            // - not found better
            // - frameCenter and pointFrom are feasible
            // - new eval Point is feasible
            // - new eval Point is not better than frameCenter
            if ( foundBetter )
            {
                continue;
            }
            
            if ( ! frameCenter.isEvalOk(evalType) || !frameCenter.isFeasible(computeType))
            {
                continue;
            }
            
            NOMAD::EvalPoint pointFromInCache;
            size_t nbFound = NOMAD::CacheBase::getInstance()->find(*pointFrom->getX(), pointFromInCache, evalType);
            if (nbFound == 0)
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearchMethod: point from not found in cache");
            }
            if ( !pointFromInCache.isEvalOk(evalType) || !pointFromInCache.isFeasible(computeType))
            {
                continue;
            }
            
            NOMAD::EvalPoint evaluatedPointFromCache;
            nbFound = NOMAD::CacheBase::getInstance()->find(*evalPoint.getX(), evaluatedPointFromCache, evalType);
            if (nbFound == 0)
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearchMethod: evaluated point not found in cache");
            }
            if ( !evaluatedPointFromCache.isEvalOk(evalType) || !evaluatedPointFromCache.isFeasible(computeType))
            {
                continue;
            }
            
            // Note: Recomputing direction, instead of using frameCenter.getDirection(),
            // to ensure we work in subspace.
            auto dirForU = NOMAD::Point::vectorize(*pointFrom->getX(), *frameCenter.getX());
            auto dirForV = NOMAD::Point::vectorize(*pointFrom->getX(), *evalPoint.getX());
            OUTPUT_INFO_START
            AddOutputInfo("Dir for u: " + dirForU.display());
            AddOutputInfo("Dir for v: " + dirForV.display());
            OUTPUT_INFO_END
            NOMAD::OutputQueue::Flush();
            
            double u = dirForU.norm(NOMAD::NormType::L2).todouble();
            double v = dirForV.norm(NOMAD::NormType::L2).todouble();
            
            // TODO maybe exception
            if (u == 0 || v == 0 || u == v)
            {
                OUTPUT_INFO_START
                s = "Line search point: u=0 or v=0 or u=v. Cannot generate point.";
                AddOutputInfo(s);
                OUTPUT_INFO_END
                return successful;
            }
            
            double f0 = pointFromInCache.getF(computeType).todouble();
            double fu = frameCenter.getF(computeType).todouble();
            double fv = evaluatedPointFromCache.getF(computeType).todouble();
            
            
            // Identify where is the minimum of the quad model of f along the direction of success (1D).
            double a = (v*(fu-f0)-u*(fv-f0))/(u*v*(u-v));
            double b = ((fu-f0)-a*u*u)/u;
            double t = -b/(2*a);
            
            // TODO maybe exception
            if (a == 0)
            {
                OUTPUT_INFO_START
                s = "Line search point: a=0. Cannot generate point.";
                AddOutputInfo(s);
                OUTPUT_INFO_END
                return successful;
            }
            if (b == 0)
            {
                OUTPUT_INFO_START
                s = "Line search point: b=0. Same as frame center.";
                AddOutputInfo(s);
                OUTPUT_INFO_END
                return successful;
            }

            
            NOMAD::Double alpha = t/u;
            
            auto dirLS = dirForU ;
            dirLS *=alpha;
            
            OUTPUT_INFO_START
            AddOutputInfo("Dir for LS: " + dirLS.display());
            OUTPUT_INFO_END
            
            
            // Generate
            auto evalPointLS = NOMAD::EvalPoint(*pointFrom->getX() + dirLS);
            
            // Insert the point
            evalPointLS.setPointFrom(std::make_shared<NOMAD::EvalPoint>(frameCenter), NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
            evalPointLS.addGenStep(getStepType());
            
            if (snapPointToBoundsAndProjectOnMesh(evalPointLS, _lb, _ub))
            {

                bool inserted = insertTrialPoint(evalPointLS);
                
                OUTPUT_INFO_START
                s = "Line search point:";
                s += (inserted) ? " inserted " : " not inserted: ";
                s += evalPointLS.display();
                AddOutputInfo(s);
                OUTPUT_INFO_END
            }
            
            if ( ! _stopReasons->checkTerminate() )
            {
                successful = evalTrialPoints(this);
            }

            // From IterationUtils. Update megaIteration barrier.
            postProcessing();
            clearTrialPoints();
            
            NOMAD::EvalPoint evaluatedPointLSFromCache;
            NOMAD::CacheBase::getInstance()->find(*evalPointLS.getX(), evaluatedPointLSFromCache, evalType);
            double ft = evaluatedPointLSFromCache.getF(computeType).todouble();
            
            OUTPUT_DEBUGDEBUG_START
            AddOutputError("point from: " + pointFromInCache.display());
            AddOutputError("Frame center: " + frameCenter.display());
            AddOutputError("Last failed point: " + evaluatedPointFromCache.display());
            AddOutputError("Line search point: " + evaluatedPointLSFromCache.display());
            AddOutputError("u: " + std::to_string(u));
            AddOutputError("v: " + std::to_string(v));
            AddOutputError("t: " + std::to_string(t));
            AddOutputError("f(0): " + std::to_string(f0) );
            AddOutputError("f(u): " + std::to_string(fu) );
            AddOutputError("f(v): " + std::to_string(fv) );
            AddOutputError("f(t): " + std::to_string(ft) );
            if (successful)
            {
                AddOutputError("Line search success" );
            }
            else
            {
                AddOutputError("Line search NOT success" );
            }
            NOMAD::OutputQueue::Flush();
            OUTPUT_DEBUGDEBUG_END
            
            
            // TODO set StopType
        }
    }

    // return true if we have a partial or full success.
    return successful;
}


void NOMAD::SimpleLineSearchMegaIteration::display( std::ostream& os ) const
{
    NOMAD::MegaIteration::display(os);
}


void NOMAD::SimpleLineSearchMegaIteration::read(  std::istream& is )
{
    // Set up structures to gather member info
    size_t k=0;
    // Read line by line
    std::string name;
    while (is >> name && is.good() && !is.eof())
    {
        if ("ITERATION_COUNT" == name)
        {
            is >> k;
        }
        else if ("BARRIER" == name)
        {
            if (nullptr != _barrier)
            {
                is >> *_barrier;
            }
            else
            {
                std::string err = "Error: Reading a Barrier onto a NULL pointer";
                throw NOMAD::Exception(__FILE__,__LINE__,err);
            }
        }
        else
        {
            for (size_t i = 0; i < name.size(); i++)
            {
                is.unget();
            }
            break;
        }
    }

    setK(k);
}

void NOMAD::SimpleLineSearchMegaIteration::generateTrialPointsImp()
{
    throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearch: cannot call generate trial points for a mega search poll.");
}


std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::SimpleLineSearchMegaIteration& megaIteration)
{
    megaIteration.display ( os );
    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::SimpleLineSearchMegaIteration& megaIteration)
{

    megaIteration.read( is );
    return is;

}
