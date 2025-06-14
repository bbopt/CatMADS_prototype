
#include "../Algos/EvcInterface.hpp"
#include "../Algos/QuadModel/QuadModelEvaluator.hpp"
#include "../Algos/AlgoStopReasons.hpp"
#include "../Algos/SubproblemManager.hpp"
#include "../Algos/SurrogateEvaluation.hpp"
#include "../Algos/SurrogateEvaluator.hpp"
#include "../Output/OutputQueue.hpp"

void NOMAD::SurrogateEvaluation::init()
{
    
    if (EvalType::SURROGATE == _evalType)
    {
        setStepType(NOMAD::StepType::SURROGATE_EVALUATION);
    }
    else if (EvalType::MODEL == _evalType)
    {
        setStepType(NOMAD::StepType::MODEL_EVALUATION);
    }
    
    
    verifyParentNotNull();
    
    // For now, no need to reset evaluator. SurrogateEvaluation is build from new every time.
}


void NOMAD::SurrogateEvaluation::startImp()
{
    if (_trialPoints.empty())
    {
        OUTPUT_INFO_START
        std::string s = "Cannot create QuadModelEvaluator with empty trial points.";
        AddOutputInfo(s);
        OUTPUT_INFO_END
        return;
    }
    
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    if (EvalType::SURROGATE == _evalType)
    {
        // BB evaluator will be selected later, once done with surrogate evaluator. See run function.
        evc->setCurrentEvaluatorType(_evalType);
        _evaluatorIsReady = true;
    }
    if(EvalType::MODEL == _evalType)
    {
        const auto& modelDisplay = _runParams->getAttributeValue<std::string>("QUAD_MODEL_DISPLAY");
        
        auto fullFixedVar = NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this);
        OUTPUT_INFO_START
        std::string s = "Create QuadModelEvaluator with fixed variable = ";
        s += fullFixedVar.display();
        AddOutputInfo(s);
        OUTPUT_INFO_END
        
        _quadModelIteration = std::make_unique<NOMAD::QuadModelIteration>(_parentStep, nullptr, 0, nullptr, _trialPoints /* trial points are used for sorting. */);
        _quadModelIteration->start();
        
        // No Run of QuadModelIteration in this case.
        
        // QuadModelIteration is used for holding the model used to create the evaluator
        auto model = _quadModelIteration->getModel();
        if (nullptr!=model && model->is_ready())
        {
            auto evaluator = std::make_shared<NOMAD::QuadModelEvaluator>(evc->getCurrentEvalParams(),
                                                                         model,
                                                                         modelDisplay,
                                                                         fullFixedVar);
            // If a quad model evaluator already exists in evc, it will be replaced by this new one. The last added evaluator is used as the currentEvaluator. Once done, BB evaluator must be selected (see below in run).
            evc->addEvaluator(evaluator);
            _evaluatorIsReady = true;
        }
        else
        {
            _evaluatorIsReady = false;
        }
        _quadModelIteration->end();
        
    }
    
    
}


bool NOMAD::SurrogateEvaluation::runImp()
{
    // Evaluation using static surrogate or model. The evaluation will be used for sorting afterward.
    // Setup evaluation for SURROGATE or MODEL (if model is ready):
    //  - Set opportunistic evaluation to false
    //  - Set the Evaluator to SURROGATE or MODEL
    //  - The point's evalType will be set to SURROGATE or MODEL in evalTrialPoints().
    // Evaluate the points using the surrogate or model
    // Reset for BB:
    //  - Reset opportunism
    //  - Reset Evaluator to BB
    // And proceed - the sort using surrogate or model will be done afterward.
    
    if (!_evaluatorIsReady)
    {
        return false;
    }
    
    NOMAD::EvcInterface evcInterface(_parentStep);
    
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    
    auto previousOpportunism = evc->getOpportunisticEval();
    evc->setOpportunisticEval(false);
    
    // No barrier need for surrogate (MODEL or SURROGATE) evaluations for sort because no comparison
    // is needed to detect success
    evc->setBarrier(nullptr);
    
    // Lock the eval point queue before adding trial point in it
    evc->lockQueue();
    
    // Keep trial points that need eval and put them in the evaluation queue
    evcInterface.keepPointsThatNeedEval(_trialPoints, false /* do not use the mesh */);
    
    // The number of points in the queue for evaluation
    // Watch out, this number must be fetched before unlocking the queue,
    // otherwise there is a risk that the evaluations already restarted
    // and the queue may be empty.
    size_t nbEvalPointsThatNeedEval = evc->getQueueSize(NOMAD::getThreadNum());
    
    // Update trial points with evaluated trial points.
    // Note: If cache is not used, Points that are already evaluated
    // will be forgotten.
    NOMAD::EvalPointSet evalPointSet;
   
    // Retrieve evaluated points from cache that should not be evaluated again.
    // This must be done BEFORE starting evaluation (this is because of parallel mode). We unlock the queue after that.
    // NOTE: There are two reasons why nbEvalPointsThatNeedEval < _trialPoints.size: 1- Trial point(s) has already been evaluated, 2- Trial point(s) is already in the queue. We are interested only in the evaluated points from cache.
    // NOTE: We can have duplicates in trial points. Insertion in queue may detect a duplicate when a mesh is extremely fine. We keep just one.
    if (nbEvalPointsThatNeedEval < _trialPoints.size())
    {
        for (const auto & evalPoint : evcInterface.retrieveEvaluatedPointsFromCache(_trialPoints))
        {
            evalPointSet.insert(evalPoint);
        }
        OUTPUT_DEBUG_START
        std::string s;
        s = "The number of points that need eval is smaller than the number of trial points. Some evaluated points are already in cache.";
        _parentStep->AddOutputDebug(s);
        OUTPUT_DEBUG_END
        
    }
    
    // Arguments to unlockQueue:
    // false: do not sort
    // keepN: default, keep all points
    evc->unlockQueue(false);
    
    // Start surrogate evaluation if needed
    if (nbEvalPointsThatNeedEval > 0)
    {
        evcInterface.startEvaluation();
        
        for (const auto & evalPoint : evcInterface.retrieveAllEvaluatedPoints())
        {
            evalPointSet.insert(evalPoint);
        }
    }

    
    OUTPUT_DEBUG_START
    std::string s;
    s = "Number of trial points: " + std::to_string(_trialPoints.size());
    _parentStep->AddOutputDebug(s);
    s = "Number of trial points that needed eval: " + std::to_string(nbEvalPointsThatNeedEval);
    _parentStep->AddOutputDebug(s);
    s = "Number of evaluated points: " + std::to_string(evalPointSet.size());
    _parentStep->AddOutputDebug(s);
    if (_trialPoints.size() != evalPointSet.size())
    {
        s = "Warning: number of trial points != number of evaluated points. This is normal if it happens just before reaching max_bb_eval.";
        _parentStep->AddOutputDebug(s);
    }
    OUTPUT_DEBUG_END
    
    
    
    _trialPoints.clear();
    _trialPoints = evalPointSet;
    
    
    evc->setOpportunisticEval(previousOpportunism);
    evc->setCurrentEvaluatorType(NOMAD::EvalType::BB);
    
    // Points are now all evaluated using SURROGATE or MODEL.
    // Points are still in the parent step's trial points. There is no update to do here.
    // They are ready to be sorted using their surrogate values, and then
    // evaluated using the BB.
    
    return true;
}


void NOMAD::SurrogateEvaluation::endImp()
{
}
