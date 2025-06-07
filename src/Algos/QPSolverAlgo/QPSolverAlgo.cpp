
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Eval/ProgressiveBarrier.hpp"
#include "../../Util/fileutils.hpp"

// QPSolver algo specifics
#include "../../Algos/QPSolverAlgo/QPSolverAlgo.hpp"
#include "../../Algos/QPSolverAlgo/QPSolverAlgoMegaIteration.hpp"

// QuadModel specifics
#include "../../Algos/QuadModel/QuadModelInitialization.hpp"


void NOMAD::QPSolverAlgo::init()
{
    setStepType(NOMAD::StepType::ALGORITHM_QPSOLVER);

    bool qpsolverAlgoOpt = _runParams->getAttributeValue<bool>("QP_OPTIMIZATION"); // true if standalone
    
    if (!qpsolverAlgoOpt)
    {
        throw NOMAD::InvalidParameter(__FILE__,__LINE__,"QP algo is intended for standalone optimization. Set QP_OPTIMIZATION true.");
    }
    _initialization = std::make_unique<NOMAD::QuadModelInitialization>(this);

}

bool NOMAD::QPSolverAlgo::runImp()
{
    _algoSuccessful = false;
    

    if ( ! _stopReasons->checkTerminate() )
    {
        size_t k = 0;   // Iteration number
        
        // Barrier was computed by Initialization (if X0 provided).
        // Barrier is used for MegaIteration management.
        std::shared_ptr<NOMAD::BarrierBase> barrier = _initialization->getBarrier();
        if (nullptr == barrier)
        {
            // Barrier constructor automatically finds the best points in the cache.
            
            auto hMax = _runParams->getAttributeValue<NOMAD::Double>("H_MAX_0");
            auto hNormType = _runParams->getAttributeValue<NOMAD::HNormType>("H_NORM");
            
            // ChT TODO check. The compute type can be DMULTI_COMBINE_F. This can be supplied by evaluator control. Can this be handled by QP solver ?
            // Compute type for this optim
            FHComputeTypeS computeType; // Default from struct initializer
            computeType.hNormType = hNormType; // REM: No PhaseOne search for this algo!
        
            // Eval type for this optim
            auto evalType = NOMAD::EvcInterface::getEvaluatorControl()->getCurrentEvalType();
            
            // Create a single objective progressive barrier
            barrier = std::make_shared<NOMAD::ProgressiveBarrier>(hMax,
                                                       NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this),
                                                       evalType,
                                                       computeType);
        }

        // Create a single MegaIteration: manage multiple iterations.
        NOMAD::QPSolverAlgoMegaIteration megaIteration(this, k, barrier,NOMAD::SuccessType::UNDEFINED);
        while (!_termination->terminate(k))
        {
            megaIteration.start();
            bool currentMegaIterSuccess = megaIteration.run();
            megaIteration.end();

            _algoSuccessful = _algoSuccessful || currentMegaIterSuccess;

            k       = megaIteration.getK();
            // NOMAD::SuccessType megaIterSuccess = megaIteration.getSuccessType();
            
            if (getUserInterrupt())
            {
                hotRestartOnUserInterrupt();
            }
        }

        // _refMegaIteration is used for hot restart (read
        // and write), as well as to keep values used in Mads::end(). Update it here.
        _refMegaIteration = std::make_shared<NOMAD::QPSolverAlgoMegaIteration>(this, k, barrier, _success);

        _termination->start();
        _termination->run();
        _termination->end();
    }

    return _algoSuccessful;
}

void NOMAD::QPSolverAlgo::readInformationForHotRestart()
{
    // Restart from where we were before.
    // For this, we need to read some files.
    // Note: Cache file is treated independently from hot restart file.

    if (_runParams->getAttributeValue<bool>("HOT_RESTART_READ_FILES"))
    {
        // Verify the files exist and are readable.
        const std::string& hotRestartFile = _runParams->getAttributeValue<std::string>("HOT_RESTART_FILE");
        if (NOMAD::checkReadFile(hotRestartFile))
        {
            std::cout << "Read hot restart file " << hotRestartFile << std::endl;

            auto barrier = _initialization->getBarrier();
            int k = 0;
            NOMAD::SuccessType success = NOMAD::SuccessType::UNDEFINED;

            _refMegaIteration = std::make_shared<NOMAD::QPSolverAlgoMegaIteration>(this, k, barrier, success);

            // Here we use QPSolverAlgo::operator>>
            NOMAD::read<QPSolverAlgo>(*this, hotRestartFile);
        }
    }
}
