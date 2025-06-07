
#include "../../Algos/EvcInterface.hpp"
#include "../../Math/LHS.hpp"
#include "../../Math/RNG.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoInitialization.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Eval/ProgressiveBarrier.hpp"


void NOMAD::TemplateAlgoInitialization::init()
{
    _templateAlgoStopReason = NOMAD::AlgoStopReasons<NOMAD::RandomAlgoStopType>::get( _stopReasons );
}


bool NOMAD::TemplateAlgoInitialization::runImp()
{
    bool doContinue = ! _stopReasons->checkTerminate();

    if (doContinue)
    {
        // Evaluate the initial trial points (x0) if available.
        evalTrialPoints(this);
        
        // Will continue, even if X0 is not available. No need to stop, this algo randomly generates points!
        doContinue = ! _stopReasons->checkTerminate();
        if ( ! doContinue )
            _templateAlgoStopReason->set(NOMAD::RandomAlgoStopType::INITIAL_FAILED);

    }
    return doContinue;
}

void NOMAD::TemplateAlgoInitialization::startImp()
{

    if ( ! _stopReasons->checkTerminate() )
    {
        // For a standalone random algo optimization (RANDOM_ALGO_OPTIMIZATION true), initial trial points are provided in x0. Else, simply pass.
        auto templateAlgo_opt = _runParams->getAttributeValue<bool>("RANDOM_ALGO_OPTIMIZATION");
        if ( templateAlgo_opt )
        {
            generateTrialPoints();
        }
        else
        {
            OUTPUT_INFO_START
            AddOutputInfo("No initialization required.");
            NOMAD::OutputQueue::Flush();
            OUTPUT_INFO_END
        }
    }
}


void NOMAD::TemplateAlgoInitialization::endImp()
{
    // Construct _barrier member with evaluated _trialPoints for future use (by Algo)
    // _trialPoints are already updated with Evals.
    // NOTE: trial point should not be empty -> otherwise the barrier is nullptr (exception thrown later)
    if (!_trialPoints.empty())
    {
        std::vector<NOMAD::EvalPoint> evalPointList;
        std::copy(_trialPoints.begin(), _trialPoints.end(),
                          std::back_inserter(evalPointList));
        auto hMax = _runParams->getAttributeValue<NOMAD::Double>("H_MAX_0");
        
        // Compute type for this optim (eval type, compute type and h norm type)
        auto hNormType = _runParams->getAttributeValue<NOMAD::HNormType>("H_NORM");

        // Eval type for this optim (can be BB or SURROGATE)
        // REM: No PhaseOne search for this algo!
        FHComputeTypeS computeType; // Default from struct initializer
        computeType.hNormType = hNormType; // REM: No PhaseOne search for this algo!
        auto evalType = NOMAD::EvcInterface::getEvaluatorControl()->getCurrentEvalType();
        
        _barrier = std::make_shared<NOMAD::ProgressiveBarrier>(hMax,
                                NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this),
                                evalType,
                                computeType,
                                evalPointList);
    }
}


// Generate trial points from x0
void NOMAD::TemplateAlgoInitialization::generateTrialPointsImp()
{
    auto x0s = _pbParams->getAttributeValue<NOMAD::ArrayOfPoint>("X0");

    // It is ok if no x0 is provided, just pass
    if(x0s.empty() || !x0s[0].isComplete())
    {
        OUTPUT_INFO_START
        AddOutputInfo("No X0 provided, No cache. Let's generate one trial point.");
        NOMAD::OutputQueue::Flush();
        OUTPUT_INFO_END
        
        auto n = _pbParams->getAttributeValue<size_t>("DIMENSION");
        auto k = _runParams->getAttributeValue<size_t>("RANDOM_ALGO_DUMMY_FACTOR");
        if (k == NOMAD::INF_SIZE_T)
        {
            throw NOMAD::Exception(__FILE__, __LINE__, "RANDOM_ALGO_DUMMY_FACTOR cannot be INF.");
        }
        
        auto lowerBound = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND");
        auto upperBound = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND");

        if (!lowerBound.isComplete() || !upperBound.isComplete())
        {
            x0s.clear(); // x0s may not be empty (but it is not complete)
            
            // Sample randomly between -(j+1) and j+1
            for (size_t j = 0; j < n*k; j++)
            {
                NOMAD::Point point(n);
                for (size_t i = 0; i < n; i++)
                {
                    point[i] = RNG::rand(-((double)j+1.0), (double)j+1.0);
                }
                x0s.push_back(point);
            }
            
        }
        else
        {
            // Let's get random points with Latin Hypercube sampling
            NOMAD::LHS lhs(n, k*n, lowerBound, upperBound);
            x0s = lhs.Sample();
        }
    }
    else
    {
        validateX0s();
    }
    
    for (const auto & x0 : x0s)
    {
        NOMAD::EvalPoint evalPoint_x0(x0);
        
        OUTPUT_INFO_START
        AddOutputInfo("Using X0: " + evalPoint_x0.display());
        NOMAD::OutputQueue::Flush();
        OUTPUT_INFO_END
        
        insertTrialPoint(evalPoint_x0);
    }

    

}
