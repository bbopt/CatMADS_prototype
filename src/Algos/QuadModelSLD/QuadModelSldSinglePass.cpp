
#include "../../Algos/QuadModelSLD/QuadModelSldSinglePass.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldOptimize.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldUpdate.hpp"
#include "../../Cache/CacheBase.hpp"


void NOMAD::QuadModelSldSinglePass::generateTrialPointsImp ()
{
    // Select the sample points to construct the model. Use a center pt and the cache
    NOMAD::QuadModelSldUpdate update(this,emptyEvalPointSet /* No trial points */);
    
    update.start();
    bool updateSuccess = update.run();
    update.end();

    // Model Update is handled in start().
    if (!_stopReasons->checkTerminate() && updateSuccess && getModel()->check() )
    {
        // Clear model value info from cache. For each pass we suppose we have a different quadratic model and MODEL value must be re-evaluated.
        NOMAD::CacheBase::getInstance()->clearModelEval(NOMAD::getThreadNum());
        
        // Optimize to generate oracle points on this model
        // Initialize optimize member - model optimizer on sgte
        NOMAD::QuadModelSldOptimize optimize (this, _pbParams );
        
        auto previousMaxOutputLevel =  NOMAD::OutputQueue::getInstance()->getMaxOutputLevel();
        NOMAD::OutputQueue::getInstance()->setMaxOutputLevel(NOMAD::OutputLevel::LEVEL_NORMAL);
        
        optimize.start();
        // No run, the trial points are evaluated somewhere else.
        optimize.end();
        
        NOMAD::OutputQueue::getInstance()->setMaxOutputLevel(previousMaxOutputLevel);
        
        const auto& trialPts = optimize.getTrialPoints();
        
        // Manage all trial points
        for ( const auto & pt : trialPts )
        {
            
            // Need to copy to a non const eval point
            NOMAD::EvalPoint scaledPt(pt);
            getModel()->unscale(scaledPt);
            insertTrialPoint(scaledPt);
            OUTPUT_DEBUG_START
            std::string s = "Unscaled xt: " + scaledPt.display();
            AddOutputInfo(s, OutputLevel::LEVEL_DEBUG);
            OUTPUT_DEBUG_END
        }
    }

    // If everything is ok we set the stop reason.
    if (! _stopReasons->checkTerminate())
    {
        auto stopReason = NOMAD::AlgoStopReasons<NOMAD::ModelStopType>::get ( getAllStopReasons() );
        stopReason->set(NOMAD::ModelStopType::MODEL_SINGLE_PASS_COMPLETED);
    }

}
