
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Eval/ProgressiveBarrier.hpp"
#include "../../Util/fileutils.hpp"

// Template algo specifics
#include "../../Algos/TemplateAlgo/TemplateAlgo.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoInitialization.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoMegaIteration.hpp"

void NOMAD::TemplateAlgo::init()
{

    setStepType(NOMAD::StepType::ALGORITHM_RANDOM);

    // Instantiate random algorithm Initialization class (called automatically by start)
    _initialization = std::make_unique<NOMAD::TemplateAlgoInitialization>( this );
}

bool NOMAD::TemplateAlgo::runImp()
{
    _algoSuccessful = false;
    
    bool randomAlgoOpt = _runParams->getAttributeValue<bool>("RANDOM_ALGO_OPTIMIZATION");

    if ( ! _stopReasons->checkTerminate() )
    {
        size_t k = 0;   // Iteration number

        std::shared_ptr<NOMAD::BarrierBase> barrier = nullptr;

        // Handle two situations for calling this: standalone optimization or search method
        if (randomAlgoOpt)
        {
            // Barrier was computed by Initialization (if X0 provided).
            barrier = _initialization->getBarrier();            
        }
        else
        {
            // Get barrier from upper MadsMegaIteration, if available (it is the case when Mads calls this as part of a search method).
            auto madsMegaIter = getParentOfType<NOMAD::MadsMegaIteration*>(false);
            if (nullptr != madsMegaIter)
            {
                barrier = madsMegaIter->getBarrier();
            }
        }

        

        // Create a single MegaIteration: manage multiple iterations.
        NOMAD::TemplateAlgoMegaIteration megaIteration(this, k, barrier,NOMAD::SuccessType::UNDEFINED);
        while (!_termination->terminate(k))
        {
            megaIteration.start();
            bool currentMegaIterSuccess = megaIteration.run();
            megaIteration.end();

            _algoSuccessful = _algoSuccessful || currentMegaIterSuccess;

            k       = megaIteration.getK();
            NOMAD::SuccessType megaIterSuccess = megaIteration.getSuccessType();

            if (!randomAlgoOpt && megaIterSuccess !=NOMAD::SuccessType::FULL_SUCCESS) // Search method stops if not full success
            {
                auto algoStopReason = NOMAD::AlgoStopReasons<NOMAD::RandomAlgoStopType>::get ( _stopReasons );
                algoStopReason->set( NOMAD::RandomAlgoStopType::SINGLE_PASS_COMPLETED ); // This will stop iterations.
            }
            
            if (getUserInterrupt())
            {
                hotRestartOnUserInterrupt();
            }
        }

        // _refMegaIteration is used for hot restart (read
        // and write), as well as to keep values used in Mads::end(). Update it here.
        _refMegaIteration = std::make_shared<NOMAD::TemplateAlgoMegaIteration>(this, k, barrier, _success);

        _termination->start();
        _termination->run();
        _termination->end();
    }

    return _algoSuccessful;
}


void NOMAD::TemplateAlgo::readInformationForHotRestart()
{
    // Restart from where we were before.
    // For this, we need to read some files.
    // Note: Cache file is treated independently of hot restart file.

    if (_runParams->getAttributeValue<bool>("HOT_RESTART_READ_FILES"))
    {
        // Verify the files exist and are readable.
        const std::string& hotRestartFile = _runParams->getAttributeValue<std::string>("HOT_RESTART_FILE");
        if (NOMAD::checkReadFile(hotRestartFile))
        {
            std::cout << "Read hot restart file " << hotRestartFile << std::endl;
            
            auto barrier = std::make_shared<NOMAD::ProgressiveBarrier>();
            int k = 0;
            NOMAD::SuccessType success = NOMAD::SuccessType::UNDEFINED;

            _refMegaIteration = std::make_shared<NOMAD::TemplateAlgoMegaIteration>(this, k, barrier, success);

            // Here we use TemplateAlgo::operator>>
            NOMAD::read<TemplateAlgo>(*this, hotRestartFile);
        }
    }
}
