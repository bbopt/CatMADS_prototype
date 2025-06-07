
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Util/fileutils.hpp"

// Algo specifics
#include "../../Algos/DMultiMads/DMultiMads.hpp"
#include "../../Algos/DMultiMads/DMultiMadsMegaIteration.hpp"
#include "../../Algos/Mads/MadsInitialization.hpp"

void NOMAD::DMultiMads::init()
{

    setStepType(NOMAD::StepType::ALGORITHM_DMULTIMADS);

    // Instantiate algorithm Initialization class (Start function automatically called)
   // The Mads initialization manages Mesh and X0
    _initialization = std::make_unique<NOMAD::MadsInitialization>( this, true /*use Cache for barrier init*/, true /*initialization for DMultiMads*/ );
    
    if (NOMAD::Algorithm::getNbObj() < 2)
    {
        throw NOMAD::InvalidParameter(__FILE__,__LINE__,"DMultiMads is intended to solve problems with more than one objective.");
    }
}

bool NOMAD::DMultiMads::runImp()
{
    _algoSuccessful = false;
    
    if ( !_runParams->getAttributeValue<bool>("DMULTIMADS_OPTIMIZATION") )
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"DMultiMads is a standalone optimization algo. Cannot be used as a Mads search method.");
    }

    if ( ! _stopReasons->checkTerminate() )
    {
        size_t k = 0;   // Iteration number

        // DMultiMadsBarrier created during Initialization (with X0).
        std::shared_ptr<NOMAD::BarrierBase> barrier = _initialization->getBarrier();
        
        // Mesh created during Initialization
        NOMAD::MeshBasePtr initialMesh = dynamic_cast<NOMAD::MadsInitialization*>(_initialization.get())->getMesh();

        // Create a single MegaIteration: manage multiple iterations.
        NOMAD::DMultiMadsMegaIteration megaIteration(this, k, barrier, initialMesh, NOMAD::SuccessType::UNDEFINED);
        while (!_termination->terminate(k))
        {
            megaIteration.start();
            megaIteration.run();
            megaIteration.end();

            k       = megaIteration.getK();
            
            if (!_algoSuccessful && megaIteration.getSuccessType() >= NOMAD::SuccessType::FULL_SUCCESS)
            {
                _algoSuccessful = true;
            }
            
            if (getUserInterrupt())
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"DMultiMads does not currently support hot restart.");
            }
        }

        // _refMegaIteration is used for hot restart (read
        // and write), as well as to keep values used in Mads::end(). Update it here.
        _refMegaIteration = std::make_shared<NOMAD::DMultiMadsMegaIteration>(this, k, barrier, nullptr, _success);

        _termination->start();
        _termination->run();
        _termination->end();
    }

    return _algoSuccessful;
}


void NOMAD::DMultiMads::readInformationForHotRestart()
{
    // TODO ... maybe
    if (_runParams->getAttributeValue<bool>("HOT_RESTART_READ_FILES"))
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"DMultiMads does not currently support hot restart.");
    }
}
