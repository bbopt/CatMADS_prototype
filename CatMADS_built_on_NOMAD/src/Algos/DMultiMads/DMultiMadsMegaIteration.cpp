
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/DMultiMads/DMultiMadsMegaIteration.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Output/OutputDirectToFile.hpp"

void NOMAD::DMultiMadsMegaIteration::init()
{
    setStepType(NOMAD::StepType::MEGA_ITERATION);

    
    // Create a DMultiMads iteration.
    // Set current best point during the Update step: use xFeas OR xInf if XFeas is not available.
    // During MegaIteration Run step we use a single iteration object with several start, run, end for the various iterations of the algorithm.
    
    _dMultiMadsIteration = std::make_shared<NOMAD::DMultiMadsIteration>(this,
                                                                        nullptr, /* the best point will be updated before start */
                                                                        0, /* the counter will be updated at start */
                                                                        _initialMesh);
    
}


void NOMAD::DMultiMadsMegaIteration::startImp()
{
    
    if ( ! _stopReasons->checkTerminate() )
    {
    
        OUTPUT_DEBUG_START
        auto frameCenter = _dMultiMadsIteration->getFrameCenter();
        AddOutputDebug("Previous frame center: " + (frameCenter ? frameCenter->display() : "NULL"));
        OUTPUT_DEBUG_END
    }
}


bool NOMAD::DMultiMadsMegaIteration::runImp()
{
    bool successful = false;
    std::string s;

    if ( _stopReasons->checkTerminate() )
    {
        OUTPUT_DEBUG_START
        s = getName() + ": stopReason = " + _stopReasons->getStopReasonAsString() ;
        AddOutputDebug(s);
        OUTPUT_DEBUG_END
        return false;
    }

    if ( _dMultiMadsIteration == nullptr )
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "No iteration to run");
    }

    if (! _stopReasons->checkTerminate())
    {
        _dMultiMadsIteration->start();

        successful = _dMultiMadsIteration->run(); // Is this iteration successful

        _dMultiMadsIteration->end();

        if (successful)
        {
            OUTPUT_DEBUG_START
            s = getName() + ": new success " + NOMAD::enumStr(getSuccessType());
            AddOutputDebug(s);
            OUTPUT_DEBUG_END
        }
            
        if (getUserInterrupt())
        {
            hotRestartOnUserInterrupt();
        }
    }
    OUTPUT_DEBUG_START
    // Display MegaIteration's stop reason
    AddOutputDebug(getName() + " stop reason set to: " + _stopReasons->getStopReasonAsString());
    OUTPUT_DEBUG_END

    // MegaIteration is a success if either a better xFeas or
    // a dominating or partial success for xInf was found.
    // See Algorithm 12.2 from DFBO.

    // return true if we have a partial or full success.
    return successful;
}


void NOMAD::DMultiMadsMegaIteration::endImp()
{
    auto megaIterBarrier = getBarrier();
    
    const std::vector<EvalPointPtr>& xFeas = megaIterBarrier->getAllXFeas();
    bool append = false;
    for (const EvalPointPtr & ev: xFeas)
    {
        NOMAD::StatsInfo info;
        
        info.setBBO(ev->getBBO(NOMAD::EvalType::BB));
        info.setSol(*(ev->getX()));
        
        NOMAD::OutputDirectToFile::Write(info, true /* write in solution file*/, false /* do no write in history file */, append /* append in solution file */);
        append = true;
    }
    
}

void NOMAD::DMultiMadsMegaIteration::display( std::ostream& os ) const
{
    NOMAD::MegaIteration::display(os);
}


void NOMAD::DMultiMadsMegaIteration::read(  std::istream& is )
{
    
    throw NOMAD::Exception(__FILE__,__LINE__,"DMultiMadsMegaIteration is not yet available.");
//    // Set up structures to gather member info
//    size_t k=0;
//    // Read line by line
//    std::string name;
//    while (is >> name && is.good() && !is.eof())
//    {
//        if ("ITERATION_COUNT" == name)
//        {
//            is >> k;
//        }
//        else if ("BARRIER" == name)
//        {
//            if (nullptr != _barrier)
//            {
//                is >> *_barrier;
//            }
//            else
//            {
//                std::string err = "Error: Reading a Barrier onto a NULL pointer";
//                std::cerr << err;
//            }
//        }
//        else
//        {
//            for (size_t i = 0; i < name.size(); i++)
//            {
//                is.unget();
//            }
//            break;
//        }
//    }
//
//    setK(k);
}






std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::DMultiMadsMegaIteration& megaIteration)
{
    megaIteration.display ( os );
    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::DMultiMadsMegaIteration& megaIteration)
{

    megaIteration.read( is );
    return is;

}
