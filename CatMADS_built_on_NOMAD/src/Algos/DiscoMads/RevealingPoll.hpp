/**
 \file   RevealingPoll.hpp
 \brief  The DiscoMads algorithm poll step
 \author Solene Kojtych
 \see    RevealingPoll.cpp
 */
#ifndef __NOMAD_4_3_REVEALING_POLLFROMPOLL__
#define __NOMAD_4_3_REVEALING_POLLFROMPOLL__

#include <set>

#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/Mads/SearchMethodBase.hpp"
#include "../../Algos/Mads/Poll.hpp"
#include "../../nomad_nsbegin.hpp"

/// Class for RevealingPoll of DiscoMads algorithm
/**
 * The "revealing poll" in DiscoMads theory is implemented as inherited from Poll as it is required for the convergence analysis
 Generate the trial points (RevealingPoll::startImp), launch evaluation (RevealingPoll::runImp) and postprocessing (RevealingPoll::endImp).
 */
class RevealingPoll: public Poll
{
private:
#ifdef TIME_STATS
    DLL_ALGO_API static double  _pollTime;      ///< Total time spent running the poll
    DLL_ALGO_API static double  _pollEvalTime;  ///< Total time spent evaluating poll points
#endif // TIME_STATS

    size_t _nbPoints;                 ///< nb of points to generate during revealing poll
    NOMAD::Double _searchRadius;      ///< radius of the revealing poll

public:
    /// Constructor
    /**
     \param parentStep The parent of this poll step
     */
    explicit RevealingPoll(const Step* parentStep)
      : Poll(parentStep)
    {
        init();
    }
    virtual ~RevealingPoll() {}

#ifdef TIME_STATS
    /// Time stats
    static double getPollTime()       { return _pollTime; }
    static double getPollEvalTime()   { return _pollEvalTime; }
#endif // TIME_STATS


private:

    /// Helper for constructor
    void init() ;

    /// Implementation for end tasks for revealing poll.
    /**
     Call the IterationUtils::postProcessing of the points with flag to not update hmax and incumbents, except if full success
     */
    virtual void  endImp() override ;
    
    /// Generate new points to evaluate
    /**
     Implementation called by IterationUtils::generateTrialPoints.
     The trial points are obtained by:
        - adding poll directions (Poll::setPollDirections) to the poll center (frame center).
        - snapping points (and directions) to bounds.
        - projecting points on mesh.
     */
    void generateTrialPointsImp() override;


    ///Helper for generateTrialPointsImp
    // Generate random poll directions for the revealing search
    /**
     \param directions  The direction computed -- \b OUT.
     \param n           The dimension of the variable space -- \b IN.
      */
     void generateDirections(std::list<Direction> &directions, const size_t n) const;
    

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_3_REVEALING_POLLFROMPOLL__
