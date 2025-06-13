
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoMegaIteration.hpp"
#include "../../Output/OutputQueue.hpp"


void NOMAD::TemplateAlgoMegaIteration::init()
{
    setStepType(NOMAD::StepType::MEGA_ITERATION);

    // Replace barrier from the MadsMegaIteration one, if available (case if we ran this as a search method).
    auto madsMegaIter = getParentOfType<NOMAD::MadsMegaIteration*>(false);
    if (nullptr != madsMegaIter)
    {
        _barrier = madsMegaIter->getBarrier();
    }
    
    // Create a template algo iteration.
    // Set current best point during the Update step: use xFeas OR xInf if XFeas is not available.
    // During template algo we use a single iteration object with several start, run, end for the various iterations of the algorithm.
    
    _templateAlgoIteration = std::make_shared<NOMAD::TemplateAlgoIteration>(this,
                                                                            nullptr, /* the best point will be updated before start */
                                                                            0 /* the counter will be updated at start */);
    
}


void NOMAD::TemplateAlgoMegaIteration::startImp()
{
    
    if ( ! _stopReasons->checkTerminate() )
    {
    
        OUTPUT_DEBUG_START
        auto frameCenter = _templateAlgoIteration->getFrameCenter();
        AddOutputDebug("Previous frame center: " + (frameCenter ? frameCenter->display() : "NULL"));
        OUTPUT_DEBUG_END
        
        
        // Default mega iteration start tasks
        // See issue #639
        NOMAD::MegaIteration::startImp();
    }
}


bool NOMAD::TemplateAlgoMegaIteration::runImp()
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

    if ( _templateAlgoIteration == nullptr )
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "No iteration to run");
    }

    if (! _stopReasons->checkTerminate())
    {
        _templateAlgoIteration->start();

        successful = _templateAlgoIteration->run();          // Is this iteration successful

        _templateAlgoIteration->end();

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


void NOMAD::TemplateAlgoMegaIteration::display( std::ostream& os ) const
{
    NOMAD::MegaIteration::display(os);
}


void NOMAD::TemplateAlgoMegaIteration::read(  std::istream& is )
{
    // Set up structures to gather member info
    size_t k=0;
    // Read line by line
    std::string name;
    while (is >> name && is.good() && !is.eof())
    {
        if ("ITERATION_COUNT" == name)
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




std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::TemplateAlgoMegaIteration& megaIteration)
{
    megaIteration.display ( os );
    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::TemplateAlgoMegaIteration& megaIteration)
{

    megaIteration.read( is );
    return is;

}
