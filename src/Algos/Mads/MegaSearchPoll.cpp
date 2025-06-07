
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/MegaSearchPoll.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::MegaSearchPoll::init()
{
    setStepType(NOMAD::StepType::MEGA_SEARCH_POLL);
    verifyParentNotNull();

    auto megaIter = dynamic_cast<const NOMAD::MadsMegaIteration*>( _megaIterAncestor );
    if (nullptr == megaIter)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"An instance of class MegaSearch must have a MadsMegaIteration among its ancestors");
    }
    
    _poll = std::make_unique<NOMAD::Poll>(this);
    _search = std::make_unique<NOMAD::Search>(this);

}


void NOMAD::MegaSearchPoll::startImp()
{
    // Generate trial points using poll and search and merge them
    generateTrialPoints();
}


bool NOMAD::MegaSearchPoll::runImp()
{
    bool foundBetter = false;
    // Ensure no max lap for MegaSearchPoll. Also reset counter before evaluation.
    NOMAD::EvcInterface::getEvaluatorControl()->setLapMaxBbEval(NOMAD::INF_SIZE_T);
    NOMAD::EvcInterface::getEvaluatorControl()->resetLapBbEval();

    if ( ! _stopReasons->checkTerminate() )
    {
        foundBetter = evalTrialPoints(this);
    }

    return foundBetter;
}


void NOMAD::MegaSearchPoll::endImp()
{
    postProcessing();
}


// Generate new points to evaluate from Poll and Search
void NOMAD::MegaSearchPoll::generateTrialPointsImp()
{
    verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, true);

    // Generate trial points for Search (all enabled search methods) and Poll.
    // Note: Search and Poll generateTrialPoints() methods both
    // take care of verifying that the generated are on mesh, and also
    // update the "PointFrom" with the frame center.
    _search->generateTrialPoints();
    const auto& trialPointsSearch = _search->getTrialPoints();

    _poll->generateTrialPoints();
    _poll->generateTrialPointsSecondPass();

    // Add extra points to reach a given number of trial points
    // -> First: count the points that would need eval (check cache and barrier)
    _poll->countTrialPointsThatNeedEval(this);
    _poll->generateTrialPointsExtra();
    
    const auto& trialPointsPoll = _poll->getTrialPoints();

    // Merge two sets and remove duplicates
    // Naive implementation. Easier to understand - I could not make std::merge,
    // std::unique or std::set_union work fine.
    // Caveat: Multiple EvalPoints copy.
    for (const auto& point : trialPointsSearch)
    {
        insertTrialPoint(point);
    }
    for (const auto& point : trialPointsPoll)
    {
        insertTrialPoint(point);
    }
    
    // Complete trial points information for sorting before eval
    completeTrialPointsInformation();
    

}

