#ifndef __NOMAD_4_5_USERPOLLMETHOD__
#define __NOMAD_4_5_USERPOLLMETHOD__

#include "../../Algos/Mads/PollMethodBase.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class UserPollMethod: Poll method defined by user.
///  Two variants are available. A regular and a free poll.
///  For both, generated trial points are combined with other 
//   poll methods.
///  Free poll trial points are not snap to bounds or to mesh.
class UserPollMethod: public PollMethodBase
{
public:
    /// Constructor
    /**
     \param parentStep      The parent of this search step -- \b IN.
     */
    explicit UserPollMethod(const Step* parentStep,
                            const EvalPointPtr frameCenter,
                            bool isFreePoll)
    : PollMethodBase(parentStep, frameCenter, false /*not second pass*/, isFreePoll)
    {
        init();
    }

    /// Intermediate to call user callback function. Only for user free poll method. Called once when user free poll methods have generated and evaluated (opportunistically) trial points. Can be used to revert modifications on the fixed variables.
    virtual void updateEndUserPoll() override;

private:

    /// Helper for constructor
    void init();

    /// The user poll method works only in the second pass.
    /**
     - It cannot be the only poll method.
     - It cannot be combined with a two passes regular method (like ortho n+1)
     - It can accept null direction in the subspace and only modify .
     \param directions  The directions obtained for this poll -- \b OUT.
      */
    void generateUnitPollDirections(std::list<Direction> &directions, const size_t n) const override final;
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_USERPOLLMETHOD__
