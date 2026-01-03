#ifndef __NOMAD_4_5_ORTHO_N_PLUS_1_POLLMETHOD__
#define __NOMAD_4_5_ORTHO_N_PLUS_1_POLLMETHOD__

#include "../../Algos/Mads/PollMethodBase.hpp"
#include "../../nomad_nsbegin.hpp"

/// Class to perform Ortho N+1 NEG/MOD Poll.
// Ortho MADS N+1 NEG/MOD:
// 1- Generate 2N points
// 2- Sort points and keep only the first N points not already evaluated and that form a basis.
// 3- Evaluate N points
// 4- If no success found, compute N+1 th direction using NEG or MOD
// 5- Evaluate 1 point
class OrthoNPlus1PollMethod final : public PollMethodBase
{
private:
    bool _flagUseQuadOpt;  ///< Flag to use Quad model optim. for N+1 th direction. Otherwise use sum of negative directions.
    
    const EvalPointPtr _frameCenter;

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit OrthoNPlus1PollMethod(const Step* parentStep,
                                     const EvalPointPtr frameCenter,
                                   bool flagUseQuadOpt)
      : PollMethodBase(parentStep, frameCenter, true), // true: hasSecondPass
        _frameCenter(frameCenter),
        _flagUseQuadOpt(flagUseQuadOpt)
    {
        init();
    }
    
private:

    /// Helper for constructor.
    /**
     Test if the OrthoNPlus1 Poll is enabled or not and set step type.
     */
    void init();

    ///Generate 2N polls direction on a unit N-Sphere (no evaluation)
    /**
    The reduction to N directions is done by function trialPointsReduction.
     \param directions  The 2N directions obtained for this poll -- \b OUT.
     \param n           The dimension of the variable space  -- \b IN.
      */
    void generateUnitPollDirections(std::list<Direction> &directions, const size_t n) const override final;

    
    /// Compute N+1th direction and add it to the vector of directions.
    /**
     \param directions  The N+1 th direction obtained for this poll -- \b OUT.
      */
    void generateSecondPassDirections(std::list<Direction> &directions) const override final;
    
    /// Reduce the number of trial points
    /*
     This is used to obtain the N most promising points from 2N points.
     If the mesh is not finest the points are sorted first.
     */
    void trialPointsReduction() override ;
    
    /// Helper for generate second pass directions (only for ortho n+1 quad)
    /*
    \param allGenerationDirections  The first N directions obtained for this poll -- \b IN.
    \param dirSec  The N+1 th direction: IN, negative sum, OUT, result of quad model opt. or negative sum (if not SUCCESS) -- \b IN / \b OUT.
     /return flag for success of optimization
     */
    bool optimizeQuadModel(const std::vector<Direction> & allGeneratingDirs, Direction & dirSec) const;
    

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_ORTHO_N_PLUS_1_POLLMETHOD__
