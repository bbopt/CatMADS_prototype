
#include "../../Algos/CacheInterface.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Eval/ProgressiveBarrier.hpp"
#include "../../Util/fileutils.hpp"

// NM specific
#include "../../Algos/NelderMead/NM.hpp"
#include "../../Algos/NelderMead/NMInitialization.hpp"
#include "../../Algos/NelderMead/NMMegaIteration.hpp"

// DMultiMads Specific
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"

void NOMAD::NM::init()
{

    setStepType(NOMAD::StepType::ALGORITHM_NM);

    // Instantiate NM initialization class
    _initialization = std::make_unique<NOMAD::NMInitialization>( this );
}

bool NOMAD::NM::runImp()
{
    _algoSuccessful = false;

    if ( ! _stopReasons->checkTerminate() )
    {
        size_t k = 0;   // Iteration number

        std::shared_ptr<NOMAD::BarrierBase> barrier = nullptr;
        
        auto nmOpt = _runParams->getAttributeValue<bool>("NM_OPTIMIZATION");

        if (nmOpt)
        {
            // Barrier was computed by Initialization.
            barrier = _initialization->getBarrier();
        }
        else
        {
            // Get barrier from upper MegaIteration, if available.
            auto megaIter = getParentOfType<NOMAD::MegaIteration*>(false);
            if (nullptr != megaIter)
            {
                barrier = megaIter->getBarrier();
            }
            
        }
        
        NOMAD::SuccessType megaIterSuccess = NOMAD::SuccessType::UNDEFINED;
        
        // Create a MegaIteration: manage multiple iterations.
        NOMAD::NMMegaIteration megaIteration(this, k, barrier, megaIterSuccess);
        while (!_termination->terminate(k))
        {
            megaIteration.start();
            bool currentMegaIterSuccess = megaIteration.run();
            megaIteration.end();

            _algoSuccessful = _algoSuccessful || currentMegaIterSuccess;

            // For passing to _refMegaIteration
            k       = megaIteration.getK();
            megaIterSuccess = megaIteration.getSuccessType();

            if (getUserInterrupt())
            {
                hotRestartOnUserInterrupt();
            }
        }
        
        // Issue #372: For hot restart make sure to save the simplex (maybe as X0s)
        // _refMegaIteration is used for hot restart (read
        // and write), as well as to keep values used in Mads::end(). Update it here.
        _refMegaIteration = std::make_shared<NOMAD::NMMegaIteration>(this, k, barrier, megaIterSuccess);

        _termination->start();
        _termination->run();
        _termination->end();
    }

    return _algoSuccessful;
}


void NOMAD::NM::readInformationForHotRestart()
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

            // Create a GMesh and a MegaIteration with default values, to be filled
            // by istream is.
            // Issue #372: Fix potential bug with Hot Restart
            // Note: Assuming the progressive barrier read is in the same subspace as the current subspace.
            // This could be fixed if we write and read the progressive barrier in full subspace.
            
            // Create a single objective barrier
            auto barrier = std::make_shared<NOMAD::ProgressiveBarrier>();
            int k = 0;
            NOMAD::SuccessType success = NOMAD::SuccessType::UNDEFINED;


            _refMegaIteration = std::make_shared<NOMAD::NMMegaIteration>(this, k, barrier, success);

            // Here we use NM::operator>>
            NOMAD::read<NM>(*this, hotRestartFile);
        }
    }
}
