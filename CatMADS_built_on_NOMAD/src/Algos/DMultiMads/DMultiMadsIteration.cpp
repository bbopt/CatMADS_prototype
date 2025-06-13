#include <algorithm>    // For std::merge and std::unique

#include "../../nomad_platform.hpp"
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/DMultiMads/DMultiMadsExpansionIntLineSearchMethod.hpp"
#include "../../Algos/DMultiMads/DMultiMadsIteration.hpp"
#include "../../Algos/DMultiMads/DMultiMadsMegaIteration.hpp"
#include "../../Algos/DMultiMads/DMultiMadsMiddlePointSearchMethod.hpp"
#include "../../Algos/DMultiMads/DMultiMadsNMSearchMethod.hpp"
#include "../../Algos/DMultiMads/DMultiMadsQuadDMSSearchMethod.hpp"
#include "../../Algos/DMultiMads/DMultiMadsQuadModSearchMethod.hpp"


// TODO
//#ifdef TIME_STATS
//#include "../../Util/Clock.hpp"
//#endif

void NOMAD::DMultiMadsIteration::init()
{
    setStepType(NOMAD::StepType::ITERATION);
    
    _DMultiMadsAlgoUpdate = std::make_unique<NOMAD::DMultiMadsUpdate> (this);
    
    _poll = std::make_unique<NOMAD::Poll>(this);
    _search = std::make_unique<NOMAD::Search>(this);

    // 1- First (at position 10), quad model search method for DMultiMads
    auto quadDMSSearch = std::make_shared<NOMAD::DMultiMadsQuadDMSSearchMethod>(this);
    _search->insertSearchMethod(10, quadDMSSearch);
    auto qmSearch = std::make_shared<NOMAD::DMultiMadsQuadModSearchMethod>(this);
    _search->insertSearchMethod(11, qmSearch);

    // 2- Nelder-Mead (NM) search for DMultiMads (at position 12)
    auto nmSearch = std::make_shared<NOMAD::DMultiMadsNMSearchMethod>(this);
    _search->insertSearchMethod(12,nmSearch);

    // 3- Special Middle Point search method for DMultiMads (at position 13)
    auto middlePtSearch = std::make_shared<NOMAD::DMultiMadsMiddlePointSearchMethod>(this);
    _search->insertSearchMethod(13, middlePtSearch);

    // 4- Special line search method for DMultiMads.
    auto expansionLinesearch = std::make_shared<NOMAD::DMultiMadsExpansionIntLineSearchMethod>(this);
    _search->insertSearchMethod(13, expansionLinesearch);
}

void NOMAD::DMultiMadsIteration::startImp()
{
    // Update the center point (the best feasible or best infeasible) around which the trial points are generated.
    _DMultiMadsAlgoUpdate->start();
    bool updateSuccess = _DMultiMadsAlgoUpdate->run();
    _DMultiMadsAlgoUpdate->end();
    
    if ( ! updateSuccess )
    {
        auto stopReason = NOMAD::AlgoStopReasons<NOMAD::MadsStopType>::get ( getAllStopReasons() );

        // The update is not a success. If the global stop reason is not set to terminate we set a default stop reason for initialization.
        if ( !_stopReasons->checkTerminate() )
            stopReason->set( NOMAD::MadsStopType::UPDATE_FAILED);
    }
    
    // Verify mesh stop conditions.
    // For DMultiMads, the mesh associated to a frame center is used by poll and search
    // Note: Mads keeps a single mesh
    // Note: The mesh could be updated when calling setFrameCenter, but it is clearer to do it explicitly.
    auto frameCenterMesh = _frameCenter->getMesh();
    if ( nullptr != frameCenterMesh)
    {
        _mesh = frameCenterMesh;
    }
    _mesh->checkMeshForStopping(_stopReasons);
    
    OUTPUT_DEBUG_START
    AddOutputDebug("Mesh Stop Reason: " + _stopReasons->getStopReasonAsString());
    OUTPUT_DEBUG_END
}


bool NOMAD::DMultiMadsIteration::runImp()
{
    // Iteration cannot generate all points before evaluation
    verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, false);

    bool iterationSuccess = false;
    
    // 1. Search
    if ( nullptr != _search && ! _stopReasons->checkTerminate() )
    {
// TODO
// #ifdef TIME_STATS
//         double searchStartTime = NOMAD::Clock::getCPUTime();
//         double searchEvalStartTime = NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime();
// #endif // TIME_STATS
    
        _search->start();
        iterationSuccess = _search->run();
        _search->end();

// TODO
//#ifdef TIME_STATS
//        _searchTime += NOMAD::Clock::getCPUTime() - searchStartTime;
//        _searchEvalTime += NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime() - searchEvalStartTime;
//#endif // TIME_STATS
        
        if (iterationSuccess)
        {
            // If success, update MegaIteration best success type with success found. No poll will be performed.
            // If not success, poll will be performed and best success type is set after that.
            getParentOfType<NOMAD::MegaIteration*>()->setSuccessType(_search->getSuccessType());
            
            // Previous success is also updated. Next iteration will start with UNDEFINED success but the previous success variable keeps track of this.
            _previousSuccess = _search->getSuccessType();
        }

    }
    if (! _stopReasons->checkTerminate() )
    {
        if (! iterationSuccess)
        {
// TODO
//#ifdef TIME_STATS
//            double pollStartTime = NOMAD::Clock::getCPUTime();
//        double pollEvalStartTime = NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime();
//#endif // TIME_STATS

            // 2. Poll
            _poll->start();
            
            // Iteration is a success if either a better xFeas or
            // a better xInf (partial success or dominating) xInf was found.
            // See Algorithm 12.2 from DFBO.
            iterationSuccess = _poll->run();
// TODO
//#ifdef TIME_STATS
//            _pollTime += NOMAD::Clock::getCPUTime() - pollStartTime;
//            _pollEvalTime += NOMAD::EvcInterface::getEvaluatorControl()->getEvalTime() - pollEvalStartTime;
//#endif // TIME_STATS
            _poll->end();
            
            // Update MegaIteration best success type with success found.
            getParentOfType<NOMAD::MegaIteration*>()->setSuccessType(_poll->getSuccessType());
            
            // Previous success is also updated. Next iteration will start with UNDEFINED success but the previous success variable keeps track of this.
            _previousSuccess = _poll->getSuccessType();
            
        }
    }

    

    // End of the iteration: iterationSuccess is true if we have a partial or full success.
    return iterationSuccess;

}
