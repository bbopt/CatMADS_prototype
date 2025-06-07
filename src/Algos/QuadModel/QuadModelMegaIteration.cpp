
#include "../../Algos/QuadModel/QuadModelIteration.hpp"
#include "../../Algos/QuadModel/QuadModelMegaIteration.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Output/OutputQueue.hpp"


void NOMAD::QuadModelMegaIteration::init()
{

}


NOMAD::QuadModelMegaIteration::~QuadModelMegaIteration()
{
    // Clear model info from cache.
    // Very important so we don't have false info in a later MegaIteration.
    NOMAD::CacheBase::getInstance()->clearModelEval(NOMAD::getThreadNum());
}


void NOMAD::QuadModelMegaIteration::startImp()
{
    // Create an iteration for a frame center.
    // Use xIncFeas or xIncInf if XIncFeas is not available.
    // Use a single iteration object with several start, run, end for the various iterations of the algorithm.

    // See issue (feature) #384

    if ( ! _stopReasons->checkTerminate() )
    {
        // MegaIteration's barrier member is already in sub dimension.
        auto bestXIncFeas = _barrier->getCurrentIncumbentFeas();
        auto bestXIncInf  = _barrier->getCurrentIncumbentInf();

        if (nullptr != bestXIncFeas)
        {
            auto sqmIteration = std::make_shared<NOMAD::QuadModelIteration>(this, bestXIncFeas);
            _iterList.push_back(sqmIteration);

        }
        else if (nullptr != bestXIncInf)
        {
            auto sqmIteration = std::make_shared<NOMAD::QuadModelIteration>(this, bestXIncInf);
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


bool NOMAD::QuadModelMegaIteration::runImp()
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

        if (!_stopReasons->checkTerminate())
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


    // MegaIteration is a success if either a better xFeas or
    // a dominating or partial success for xInf was found.
    // See Algorithm 12.2 from DFBO.
    // return true if we have a partial or full success.
    return successful;
}

void NOMAD::QuadModelMegaIteration::endImp()
{
    // Clear model info from cache.
    // Very important so we don't have false info in a later MegaIteration.
    NOMAD::CacheBase::getInstance()->clearModelEval(NOMAD::getThreadNum());
    NOMAD::MegaIteration::endImp();
}


std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::QuadModelMegaIteration& megaIteration)
{
    megaIteration.display ( os );
    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::QuadModelMegaIteration& megaIteration)
{

    megaIteration.read( is );
    return is;

}
