#ifndef __NOMAD_4_5_POLLMETHODBASE__
#define __NOMAD_4_5_POLLMETHODBASE__

#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/Step.hpp"
#include "../../Math/Direction.hpp"
#include "../../Math/SimpleRNG.hpp"

#include "../../nomad_nsbegin.hpp"


/// Class for generic poll method of MADS. Run by Poll.
/**
 */
class PollMethodBase: public Step, public IterationUtils
{
private:
    const EvalPointPtr _frameCenter;
    const bool _hasSecondPass;      ///< A Second pass is available after first pass fail to improve. Ortho N+1 methods require second pass.
    const bool _isFreePoll;         ///< Flag to indicate that a free poll method is enabled.
    size_t _n; ///< Pb dimension

    // The poll method type is set by Poll when the poll method is created.
    bool _isPrimary = false;
    
    ArrayOfDouble _lb, _ub; ///< Pb bounds
    ListOfVariableGroup _varGroups; ///< Groups of variables

    bool _subsetListVG;  ///< The poll is for a subset of variables with dim subset < n
    
protected:
    bool _scaleAndProjectSecondPassDirectionOnMesh ; ///< Flag to scale and project on mesh

    std::shared_ptr<SimpleRNG> _rng = nullptr; ///< Random number generator
    
public:
    /// Constructor
    /**
     /param parentStep      The parent of this poll step -- \b IN.
     */
    explicit PollMethodBase(const Step* parentStep,
                            const EvalPointPtr frameCenter,
                            const bool hasSecondPass = false,
                            const bool isFreePoll = false)
      : Step(parentStep),
        IterationUtils(parentStep),
        _frameCenter(frameCenter),
        _hasSecondPass(hasSecondPass),
        _isFreePoll(isFreePoll),
        _scaleAndProjectSecondPassDirectionOnMesh(true),
        _subsetListVG(false)
    {
        init();
    }

    bool hasSecondPass() const { return _hasSecondPass; }
    bool isFreePoll() const { return _isFreePoll; }

    /// Implementation of startImp.
    /**
      Reset trial point stats.
      Point generation is done in Poll.
     */
    void startImp() override
    {
        // Reset the current counters. The total counters are not reset (done only once when constructor is called).
        _trialPointStats.resetCurrentStats();

    }

    /// Implementation of endImp.
    /**
      Do nothing.
      Evaluation is done in Poll.
     */
    bool runImp() override { return true; }

    /// Implementation of endImp
    /**
      Do nothing.
      postProcessing is done in Poll.
    */
    void endImp() override {}

    /// Set the variables groups managed by a Poll method.
    /**
     The default is to set all the groups of variables. This is done by the Poll class that create the poll methods.
     Example of use for non default. A user poll method manages exclusively a subset of variables. No other poll method must
     manage them. The remaining variables are managed by another poll methods. Two groups can be defined.
     */
    void setListVariableGroups(const ListOfVariableGroup & varGroups);

    /// Intermediate function used by generateTrialPoints
    std::list<NOMAD::Direction> generateFullSpaceScaledDirections(bool isSecondPass, const NOMAD::MeshBasePtr& mesh = nullptr);

    /// Reduce the number of trial points
    /*
     This is currently used only by Ortho Mads n+1.
     */
    virtual void trialPointsReduction() {} ;

    /// Update function called by Poll::end. Implemented only by UserPollMethod
    virtual void updateEndUserPoll() {};
    
    /// Access to the frame center
    const EvalPointPtr getFrameCenter() const { return _frameCenter; }
    
    
    /// set Primary flag
    void setIsPrimary(bool isPrimary) { _isPrimary = isPrimary; }

    /// Access to flag isPrimary
    bool isPrimary() const { return _isPrimary; }
    
    void setRandomGenerator(const std::shared_ptr<SimpleRNG>& rng) { _rng = rng; }
    
protected:
    void init();

    /// Compute 2n directions (from which n directions will be chosen).
    /// Used in Ortho 2N and in Ortho N+1.
    /**
     \param directions  The 2n directions obtained for this poll -- \b OUT.
     \param n           The dimension of the variable space  -- \b IN.
      */
    void generate2NDirections(std::list<NOMAD::Direction> &directions, size_t n) const;

private:

    /// Generate poll directions on a unitary frame. See derived classes (Ortho2nPollMethod, Np1UniPollMethod,...) for implementations.
    virtual void generateUnitPollDirections(std::list<Direction> &directions, const size_t dim) const = 0;

    /// Generate second pass directions. Optionally reimplemented (in Ortho N+1 and maybe more in the future).
    virtual void generateSecondPassDirections(std::list<Direction> &directions) const {};

    /// Private method to handle general case and also second pass generation
    /*
        Real implementation to generate the trial points.
        Snap the points to bounds and mesh.
     */
    void generateTrialPointsInternal(const bool isSecondPass = false);

    /// Scale and project on mesh poll directions.
    /**
     /param dirs      The unit directions to be scaled and projected on mesh -- \b IN/OUT.
     */
    void scaleAndProjectOnMesh(std::list<Direction> & dirs, std::shared_ptr<NOMAD::MeshBase> mesh = nullptr );

    /// Intermediate function (not yet the implementation that generates the trial points)
    /**
         - Display before and after generation comments.
         - Launch the implementation of the poll method to generate the trial points (::generateTrialPointsInternal).
    */
    void generateTrialPointsImp() override ;

    /// Intermediate function to compute second pass trial points.
    void generateTrialPointsSecondPassImp() override ;

    /// Implementation to increment the nb of calls counter
    virtual void incrementCounters() override { _trialPointStats.incrementNbCalls() ;}

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_POLLMETHODBASE__
