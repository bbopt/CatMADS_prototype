
#include "../../Algos/Mads/MadsInitialization.hpp"
#include "../../Algos/SSDMads/SSDMads.hpp"
#include "../../Algos/SSDMads/SSDMadsMegaIteration.hpp"


void NOMAD::SSDMads::init()
{
    setStepType(NOMAD::StepType::ALGORITHM_SSD_MADS);
    verifyParentNotNull();

    // Instantiate Mads initialization class
    _initialization = std::make_unique<NOMAD::MadsInitialization>( this );
}


bool NOMAD::SSDMads::runImp()
{
    size_t k = 0;   // Iteration number
    NOMAD::SuccessType megaIterSuccess = NOMAD::SuccessType::UNDEFINED;

    bool runOk = true;

    // Note: _initialization is run in Algorithm::startImp().

    if (!_termination->terminate(k))
    {
        std::shared_ptr<NOMAD::MeshBase> mesh = dynamic_cast<NOMAD::MadsInitialization*>(_initialization.get())->getMesh();


        const auto& barrier = _initialization->getBarrier();

        // Mads member _megaIteration is used for hot restart (read and write),
        // as well as to keep values used in Mads::end(), and may be used for _termination.
        // Update it here.
        _refMegaIteration = std::make_shared<NOMAD::SSDMadsMegaIteration>(this, k, barrier, mesh, megaIterSuccess);

        // Create a MegaIteration to manage the pollster worker and the regular workers.
        NOMAD::SSDMadsMegaIteration ssdMegaIteration(this, k, barrier, mesh, megaIterSuccess);
        while (!_termination->terminate(k))
        {

            ssdMegaIteration.start();
            ssdMegaIteration.run();
            ssdMegaIteration.end();

            k       = ssdMegaIteration.getK();
            megaIterSuccess = ssdMegaIteration.getSuccessType();

            if (getUserInterrupt())
            {
                hotRestartOnUserInterrupt();
            }
        }
    }

    else
    {
        runOk = false;
    }

    _termination->start();
    _termination->run();
    _termination->end();

    return runOk;
}
