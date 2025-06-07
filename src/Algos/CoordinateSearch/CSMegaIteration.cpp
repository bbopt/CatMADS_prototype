
#include "../../Algos/CoordinateSearch/CS.hpp"
#include "../../Algos/CoordinateSearch/CSMegaIteration.hpp"
#include "../../Algos/CoordinateSearch/CSUpdate.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::CSMegaIteration::init()
{
    setStepType(NOMAD::StepType::MEGA_ITERATION);
    
    // Create a single CSIteration
    _csIteration = std::make_unique<NOMAD::CSIteration>(this, _k, _mainMesh);
}



void NOMAD::CSMegaIteration::startImp()
{
    // Update main mesh and barrier.
    NOMAD::CSUpdate update( this );
    update.start();
    update.run();
    update.end();

    // Verify mesh stop conditions.
    _mainMesh->checkMeshForStopping( _stopReasons );

    OUTPUT_DEBUG_START
    AddOutputDebug("Mesh Stop Reason: " + _stopReasons->getStopReasonAsString());
    OUTPUT_DEBUG_END
}

bool NOMAD::CSMegaIteration::runImp()
{

    std::string s;

    if ( _stopReasons->checkTerminate() )
    {
        OUTPUT_DEBUG_START
        s = "MegaIteration: stopReason = " + _stopReasons->getStopReasonAsString() ;
        AddOutputDebug(s);
        OUTPUT_DEBUG_END
        return false;
    }

    // Get CS ancestor to call terminate(k)
    NOMAD::CS* cs = getParentOfType<NOMAD::CS*>();
    if (nullptr == cs)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "CS MegaIteration without CS ancestor");
    }
    if (!cs->terminate(_k))
    {

        OUTPUT_DEBUG_START
        AddOutputDebug("Iteration generated:");
        AddOutputDebug(_csIteration->getName());
        NOMAD::ArrayOfDouble meshSize  = _csIteration->getMesh()->getdeltaMeshSize();
        NOMAD::ArrayOfDouble frameSize = _csIteration->getMesh()->getDeltaFrameSize();
        AddOutputDebug("Mesh size:  " + meshSize.display());
        AddOutputDebug("Frame size: " + frameSize.display());
        OUTPUT_DEBUG_END

        _csIteration->start();
        bool iterSuccessful = _csIteration->run();
        _csIteration->end();

        if (iterSuccessful)
        {
            OUTPUT_DEBUG_START
            s = getName() + ": new success " + NOMAD::enumStr(_success);
            AddOutputDebug(s);
            OUTPUT_DEBUG_END
        }

        // Update MegaIteration's stop reason
        if (_stopReasons->checkTerminate())
        {
            OUTPUT_DEBUG_START
            s = getName() + " stop reason set to: " + _stopReasons->getStopReasonAsString();
            AddOutputDebug(s);
            OUTPUT_DEBUG_END
        }

        // Note: Delta (frame size) will be updated in the Update step next time it is called. not sure if kept

        if (getUserInterrupt())
        {
            hotRestartOnUserInterrupt();
        }
    }

    // MegaIteration is a success if either a better xFeas or
    // a dominating or partial success for xInf was found.
    // See Algorithm 12.2 from DFBO.

    // return true if we have a partial or full success.
    return (_success >= NOMAD::SuccessType::PARTIAL_SUCCESS);
}

void NOMAD::CSMegaIteration::display(std::ostream& os) const
{
    os << "MAIN_MESH " << std::endl;
    os << *_mainMesh ;
    NOMAD::MegaIteration::display(os);
}

void NOMAD::CSMegaIteration::read(std::istream& is)
{
    // Set up structures to gather member info
    size_t k = 0;
    // Read line by line
    std::string name;
    while (is >> name && is.good() && !is.eof())
    {
        if ("MAIN_MESH" == name)
        {
            if (nullptr != _mainMesh)
            {
                is >> *_mainMesh;
            }
            else
            {
                std::string err = "Error: Reading a mesh onto a NULL pointer";
                throw NOMAD::Exception(__FILE__,__LINE__, err);
            }
        }
        else if ("ITERATION_COUNT" == name)
        {
            is >> k;
        }
        else if ("BARRIER" == name)
        {
            if (nullptr != _barrier)
            {
                is >> *_barrier;
            }
            else
            {
                std::string err = "Error: Reading a Barrier onto a NULL pointer";
                throw NOMAD::Exception(__FILE__,__LINE__, err);
            }
        }
        else
        {
            for (size_t i = 0; i < name.size(); i++)
            {
                is.unget();
            }
            break;
        }
    }

    setK(k);
}


std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::CSMegaIteration& megaIteration)
{
    megaIteration.display ( os );
    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::CSMegaIteration& megaIteration)
{
    megaIteration.read(is);
    return is;
}
