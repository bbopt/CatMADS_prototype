
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/NelderMead/NMMegaIteration.hpp"
#include "../../Output/OutputQueue.hpp"


void NOMAD::NMMegaIteration::init()
{
    setStepType(NOMAD::StepType::MEGA_ITERATION);

    // Get barrier from upper MegaIteration, if available.
    if (nullptr == _barrier)
    {
        auto megaIter = getParentOfType<NOMAD::MegaIteration*>(false);
        if (nullptr != megaIter)
        {
            _barrier = megaIter->getBarrier();
        }
    }
}


void NOMAD::NMMegaIteration::startImp()
{
    // Create a Nelder Mead iteration for a simplex center.
    // Use xIncFeas or xIncInf if XIncFeas is not available.
    // During NM, we use a single iteration object with several start, run, end for the various iterations of the algorithm.

    if ( ! _stopReasons->checkTerminate() )
    {
        // MegaIteration's barrier member is already in sub dimension.
        auto bestXFeas = _barrier->getCurrentIncumbentFeas();
        auto bestXInf  = _barrier->getCurrentIncumbentInf();

        // Note: getParentOfType with argument "false" gets over the "Algorithm" parents.
        // Here, we are looking for a MegaIteration which would be ancestor of
        // the NM (Algorithm) parent.
        auto megaIter = getParentOfType<NOMAD::MegaIteration*>(false);
        std::shared_ptr<NOMAD::MeshBase> mesh = nullptr;

        if ( megaIter != nullptr )
        {
            mesh = megaIter->getMesh();
        }

        if (nullptr != bestXFeas)
        {
            _nmIteration = std::make_shared<NOMAD::NMIteration>(this,
                                    std::make_shared<NOMAD::EvalPoint>(*bestXFeas),
                                    _k,
                                    mesh);
            _k++;
        }
        else if (nullptr != bestXInf)
        {
            _nmIteration = std::make_shared<NOMAD::NMIteration>(this,
                                    std::make_shared<NOMAD::EvalPoint>(*bestXInf),
                                    _k,
                                    mesh);
            _k++;
        }

        OUTPUT_DEBUG_START
        auto simplexCenter = _nmIteration->getSimplexCenter();
        AddOutputDebug("Simplex center: " + simplexCenter->display());
        auto previousSimplexCenter = simplexCenter->getPointFrom();
        AddOutputDebug("Previous simplex center: " + (previousSimplexCenter ? previousSimplexCenter->display() : "NULL"));
        OUTPUT_DEBUG_END
        
        // Default mega iteration start tasks
        // See issue #639
        NOMAD::MegaIteration::startImp();
    }
}


bool NOMAD::NMMegaIteration::runImp()
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

    if ( _nmIteration == nullptr )
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "No iteration to run");
    }


    const auto maxIter = (size_t)NOMAD::D_INT_MAX; // Could be a parameter.
    size_t nbMegaIter = 0;
    while ( ! _stopReasons->checkTerminate() && nbMegaIter < maxIter )
    {
        _nmIteration->start();
        bool iterSuccessful = _nmIteration->run();          // Is this iteration successful
        _nmIteration->end();
        
        successful = iterSuccessful || successful;  // Is the whole MegaIteration successful

        if (iterSuccessful)
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

        nbMegaIter++;
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


void NOMAD::NMMegaIteration::display( std::ostream& os ) const
{
    NOMAD::MegaIteration::display(os);
}


void NOMAD::NMMegaIteration::read(  std::istream& is )
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


std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::NMMegaIteration& megaIteration)
{
    megaIteration.display ( os );
    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::NMMegaIteration& megaIteration)
{

    megaIteration.read( is );
    return is;

}
