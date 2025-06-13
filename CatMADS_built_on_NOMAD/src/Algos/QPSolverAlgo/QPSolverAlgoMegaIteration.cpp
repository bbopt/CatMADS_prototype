
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/QPSolverAlgo/QPSolverAlgoMegaIteration.hpp"
#include "../../Output/OutputQueue.hpp"


void NOMAD::QPSolverAlgoMegaIteration::init()
{
    setStepType(NOMAD::StepType::MEGA_ITERATION);

    // Replace barrier from the MadsMegaIteration one, if available (case if we ran this as a search method).
    auto madsMegaIter = getParentOfType<NOMAD::MadsMegaIteration*>(false);
    if (nullptr != madsMegaIter)
    {
        _barrier = madsMegaIter->getBarrier();
    }
}

void NOMAD::QPSolverAlgoMegaIteration::startImp()
{

    // Create an iteration for a frame center.
    // Use xFeas or xInf if XFeas is not available.
    // Use a single iteration object with several start, run, end for the various iterations of the algorithm.
    _iterList.clear();
    
    if ( ! _stopReasons->checkTerminate() )
    {
        auto bestXFeas = _barrier->getCurrentIncumbentFeas();
        auto bestXInf = _barrier->getCurrentIncumbentInf();

        if (nullptr != bestXFeas)
        {
            auto sqmIteration = std::make_shared<NOMAD::QPSolverAlgoIteration>(this, bestXFeas);
            _iterList.push_back(sqmIteration);
            
        }
        else if (nullptr != bestXInf)
        {
            auto sqmIteration = std::make_shared<NOMAD::QPSolverAlgoIteration>(this, bestXInf);
            _iterList.push_back(sqmIteration);

        }

        size_t nbIter = _iterList.size();


        AddOutputInfo(getName() + " has " + NOMAD::itos(nbIter) + " iteration" + ((nbIter > 1)? "s" : "") + ".");
    
        AddOutputDebug("Iterations generated:");
        for (size_t i = 0; i < nbIter; i++)
        {
            auto sqmIteration = _iterList[i];
            if ( sqmIteration == nullptr )
            {
                throw NOMAD::Exception(__FILE__, __LINE__, "Invalid shared pointer");
            }

            AddOutputDebug( _iterList[i]->getName());
            // Ensure we get frame center from a QuadModelIteration.
            auto frameCenter = sqmIteration->getRefCenter();
            AddOutputDebug("Frame center: " + frameCenter->display());
            auto previousFrameCenter = frameCenter->getPointFrom();
            AddOutputDebug("Previous frame center: " + (previousFrameCenter ? previousFrameCenter->display() : "NULL"));

            if (nullptr != sqmIteration->getMesh())
            {
                NOMAD::ArrayOfDouble meshSize  = sqmIteration->getMesh()->getdeltaMeshSize();
                NOMAD::ArrayOfDouble frameSize = sqmIteration->getMesh()->getDeltaFrameSize();

                AddOutputDebug("Mesh size:  " + meshSize.display());
                AddOutputDebug("Frame size: " + frameSize.display());
            }

            NOMAD::OutputQueue::Flush();
        }
    }
}


bool NOMAD::QPSolverAlgoMegaIteration::runImp()
{
    bool successful = false;
    std::string s;

    if (_iterList.empty())
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "No iterations to run");
    }

    for (size_t i = 0; i < _iterList.size(); i++)
    {
        auto sqmIteration = _iterList[i];
        if ( sqmIteration == nullptr )
        {
            throw NOMAD::Exception(__FILE__, __LINE__, "No iteration to run");
        }

        // if ( _stopReasons->checkTerminate() )
        {
            sqmIteration->start();

            bool iterSuccessful = sqmIteration->run();          // Is this iteration successful
            successful = iterSuccessful || successful;  // Is the whole MegaIteration successful

            sqmIteration->end();

            if (iterSuccessful)
            {
                s = getName() + ": new success " + NOMAD::enumStr(getSuccessType());
                AddOutputDebug(s);
            }

            if (getUserInterrupt())
            {
                hotRestartOnUserInterrupt();
            }
         }
    }

    // Display MegaIteration's stop reason
    AddOutputDebug(getName() + " stop reason set to: " + _stopReasons->getStopReasonAsString());

    // return true if we have a partial or full success.
    return successful;
}


void NOMAD::QPSolverAlgoMegaIteration::display( std::ostream& os ) const
{
    NOMAD::MegaIteration::display(os);
}


void NOMAD::QPSolverAlgoMegaIteration::read(  std::istream& is )
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
                std::cerr << err;
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




std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::QPSolverAlgoMegaIteration& megaIteration)
{
    megaIteration.display ( os );
    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::QPSolverAlgoMegaIteration& megaIteration)
{

    megaIteration.read( is );
    return is;

}
