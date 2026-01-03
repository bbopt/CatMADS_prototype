
#include "../../Algos/QuadModelSLD/QuadModelSldAlgo.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldMegaIteration.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldInitialization.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Eval/ProgressiveBarrier.hpp"
#include "../../Output/OutputQueue.hpp"

#include "../../../ext/sgtelib/src/Surrogate_Factory.hpp"
//

void NOMAD::QuadModelSldAlgo::init()
{
    setStepType(NOMAD::StepType::ALGORITHM_QUAD_MODEL);
    verifyParentNotNull();

    // Instantiate quad model initialization class
    _initialization = std::make_unique<NOMAD::QuadModelSldInitialization>(this);

}


/*-------------------------*/
/*       Destructor        */
/*-------------------------*/
NOMAD::QuadModelSldAlgo::~QuadModelSldAlgo()
{
}

bool NOMAD::QuadModelSldAlgo::runImp()
{
    bool success = false;

    size_t k = 0;   // Iteration number

    if (!_termination->terminate(k))
    {
        // Barrier constructor automatically finds the best points in the cache.
        // Barrier is used for MegaIteration management.
        
        auto barrier = _initialization->getBarrier();
        if (nullptr == barrier)
        {
            auto hMax = _runParams->getAttributeValue<NOMAD::Double>("H_MAX_0");
            
            // Compute type for this optim (eval type, compute type and h norm type)
            auto hNormType = _runParams->getAttributeValue<NOMAD::HNormType>("H_NORM");
            FHComputeTypeS computeType; // Default from struct initializer
            computeType.hNormType = hNormType; // REM: No PhaseOne search for this algo!
            
            
            // Eval type for this optim (can be BB or SURROGATE)
            auto evalType = NOMAD::EvcInterface::getEvaluatorControl()->getCurrentEvalType();
            
            barrier = std::make_shared<NOMAD::ProgressiveBarrier>(hMax,
                                                       NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this),
                                                       evalType,
                                                       computeType);
        }
        
        NOMAD::SuccessType megaIterSuccessType = NOMAD::SuccessType::UNDEFINED;
        
        // member _megaIteration is used for hot restart (read and write)
        // Update it here.
        _refMegaIteration = std::make_shared<NOMAD::QuadModelSldMegaIteration>(this, k, barrier, megaIterSuccessType);
        
        // Create an MegaIteration: manage multiple iterations around
        // different frame centers at the same time.
        NOMAD::QuadModelSldMegaIteration megaIteration(this, k, barrier, megaIterSuccessType);
        
        while (!_termination->terminate(k))
        {
            megaIteration.start();
            bool currentMegaIterSuccess = megaIteration.run();
            megaIteration.end();
            
            success = success || currentMegaIterSuccess;
            
            // Remember these values to construct the next MegaIteration.
            k       = megaIteration.getK();
            barrier = megaIteration.getBarrier();
            megaIterSuccessType = megaIteration.NOMAD::MegaIteration::getSuccessType();
            
            if (getUserInterrupt())
            {
                hotRestartOnUserInterrupt();
            }
        }
    }

    _termination->start();
    _termination->run();
    _termination->end();

    NOMAD::OutputQueue::Flush();

    return success;
}
