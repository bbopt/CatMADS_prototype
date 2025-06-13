
#include "../../Algos/QuadModel/QuadModelSinglePass.hpp"
#include "../../Algos/QuadModel/QuadModelOptimize.hpp"
#include "../../Algos/QuadModel/QuadModelUpdate.hpp"
#include "../../Cache/CacheBase.hpp"


void NOMAD::QuadModelSinglePass::generateTrialPointsImp ()
{
    
    // Select the sample points to construct the model. Use a center pt and the cache
    NOMAD::QuadModelUpdate update(this,_scalingDirections,emptyEvalPointSet /* No trial points -> for search */, _flagPriorCombineObjsForModel);
        
    update.start();
    bool updateSuccess = update.run();
    update.end();
    
    // Model Update is handled in start().
    if (!_stopReasons->checkTerminate() && updateSuccess && getModel()->is_ready() )
    {
        // Clear model value info from cache. For each pass we suppose we have a different quadratic model and MODEL value must be re-evaluated.
        NOMAD::CacheBase::getInstance()->clearModelEval(NOMAD::getThreadNum());

        // Optimize to generate oracle points on this model
        // Initialize optimize member - model optimizer on sgte
        bool scaledBounds = (!_scalingDirections.empty());
        NOMAD::QuadModelOptimize optimize (this, _pbParams , scaledBounds, _flagPriorCombineObjsForModel);

        optimize.start();
        // No run, the trial points are evaluated somewhere else.
        optimize.end();

        const auto& trialPts = optimize.getTrialPoints();
        
        // Manage all trial points
        for ( const auto & pt : trialPts )
        {
            if (scaledBounds)
            {
                // Need to copy to a non const eval point
                NOMAD::EvalPoint scaledPt(pt);
                update.unscalingByDirections(scaledPt);
                insertTrialPoint(scaledPt);
                OUTPUT_DEBUG_START
                std::string s = "Unscaled xt: " + scaledPt.display();
                AddOutputInfo(s, OutputLevel::LEVEL_DEBUG);
                OUTPUT_DEBUG_END
                
            }
            else
            {
                insertTrialPoint( pt );
            }
        }
        
        // Manage the best feasible point and best infeasible point
        _bestXFeas = optimize.getBestFeas();
        _bestXInf = optimize.getBestInf();
        if (scaledBounds)
        {
            if(nullptr != _bestXFeas)
            {
                update.unscalingByDirections(*_bestXFeas);
            }
            if(nullptr != _bestXInf)
            {
                update.unscalingByDirections(*_bestXInf);
            }
        }
    }

    // If everything is ok we set the stop reason.
    if (! _stopReasons->checkTerminate())
    {
        auto stopReason = NOMAD::AlgoStopReasons<NOMAD::ModelStopType>::get ( getAllStopReasons() );
        stopReason->set(NOMAD::ModelStopType::MODEL_SINGLE_PASS_COMPLETED);
    }

}
