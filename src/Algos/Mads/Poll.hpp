#ifndef __NOMAD_4_5_POLL__
#define __NOMAD_4_5_POLL__

#include <set>

#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/Mads/PollMethodBase.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for the poll (step 3) of MADS algorithm.
/**
 Generate the trial points (Poll::startImp), launch evaluation (Poll::runImp) and postprocessing (Poll::endImp).
 */
class Poll: public Step, public IterationUtils
{
private:
#ifdef TIME_STATS
    DLL_ALGO_API static double  _pollTime;      ///< Total time spent running the poll
    DLL_ALGO_API static double  _pollEvalTime;  ///< Total time spent evaluating poll points
#endif // TIME_STATS

    DirectionTypeList _primaryDirectionTypes, _secondaryDirectionTypes;  ///< The poll methods implement different direction types for primary and secondary poll centers.

    ListOfVariableGroup _varGroups ; ///< The variables groups to consider for the direction types.

    std::map<NOMAD::DirectionType,NOMAD::ListOfVariableGroup> _mapDirTypeToVG;  ///< Map to associate a direction type to a variable group. Only when more than one variable group is defined.

    NOMAD::Double _rho; ///< Rho parameter of the progressive barrier. Used to choose if the primary frame center is the feasible or infeasible  incumbent.

    size_t _trialPointMaxAddUp; ///< Add new trial points to the ones produced by the selected direction type up to reached a given number.

    bool _userPollMethodCallbackEnabled;


protected:
    bool _hasSecondPass;   ///<  Ortho n+1 poll methods generate n trial points in a first pass and, if not successful, generate the n+1 th point (second pass)

    bool _hasUserPollMethod; ///<  Flag to indicate that a user poll has been defined. User poll combined with regular poll methods.
    bool _hasUserFreePollMethod; ///<  Flag to indicate that a user free poll has been defined. User free poll combined with regular poll methods.

    std::vector<std::shared_ptr<PollMethodBase>> _pollMethods;  ///< Unlike for Search, Poll methods generate all their points and only then they are evaluated.

    std::vector<EvalPointPtr> _frameCenters;  ///< The frame centers (primary and secondary) of the poll methods. See createPollMethods. Refactoring required. See issue #644.

public:
    /// Constructor
    /**
     \param parentStep The parent of this poll step
     */
    explicit Poll(const Step* parentStep, bool userCallbackEnabled=false)
      : Step(parentStep),
        IterationUtils(parentStep),
        _pollMethods(),
        _userPollMethodCallbackEnabled(userCallbackEnabled)
    {
        init();
    }
    virtual ~Poll() {}



    /// Second pass of point generation after first pass is not successful.
    /**
      For Ortho N+1 methods the N+1th point is produced after evaluating N sorted points without success.
      In this second pass, the trial points are determined using the evaluations of the first N points.
      */
    void generateTrialPointsSecondPass();

    /// Extra trial point generation after first pass.
    /**
      Add trial points to reached a prescribed trial point number (TRIAL_POINT_MAX_ADD_UP).
     Only for single pass direction type (ortho 2n).
      */
    void generateTrialPointsExtra();

#ifdef TIME_STATS
    /// Time stats
    static double getPollTime()       { return _pollTime; }
    static double getPollEvalTime()   { return _pollEvalTime; }
#endif // TIME_STATS

protected:
    /// Helper for start: get lists of Primary and Secondary Polls
    void computePrimarySecondaryPollCenters(std::vector<EvalPointPtr> &primaryCenters, std::vector<EvalPointPtr> &secondaryCenters) const;

    /// Helper for start: create poll methods
    virtual void createPollMethods(const bool isPrimary, const EvalPointPtr& frameCenter);

    /// Helper for generateTrialPoints
    ///  Set the stop type for the Algorithm (can be reimplemented, for example CS)
    virtual void setMeshPrecisionStopType();

    /// Helper to create poll methods for current poll centers
    virtual void createPollMethodsForPollCenters();


private:
    /// Helper for constructor
    void init();


    /// Implementation for start tasks for MADS poll.
    /**
     Call to generate the poll methods
     */
    virtual void    startImp() override ;

    /// Implementation for run tasks for MADS poll.
    /**
     Call poll methods and perform trial points evaluation.
     \return Flag \c true if found better solution \c false otherwise.
     */
    virtual bool    runImp() override;

    /// Implementation for end tasks for MADS poll.
    /**
     Call the IterationUtils::postProcessing of the points.
     */
    virtual void    endImp() override ;

    /// Generate new points to evaluate
    /**
     Implementation called by IterationUtils::generateTrialPoints.
     The trial points are obtained by:
        - adding poll directions (Poll::setPollDirections) to the poll center (frame center).
        - snapping points (and directions) to bounds.
        - projecting points on mesh.
     */
    void generateTrialPointsImp() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_POLL__
