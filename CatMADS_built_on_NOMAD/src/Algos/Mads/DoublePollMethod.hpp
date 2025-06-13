#ifndef __NOMAD_4_5_DOUBLEPOLLMETHOD__
#define __NOMAD_4_5_DOUBLEPOLLMETHOD__

#include "../../Algos/Mads/PollMethodBase.hpp"
#include "../../nomad_nsbegin.hpp"

/// Class to perform two poll directions (a single direction and its opposite).
class DoublePollMethod final : public PollMethodBase
{
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit DoublePollMethod(const Step* parentStep,
                              const EvalPointPtr frameCenter)
      : PollMethodBase(parentStep, frameCenter)
    {
        init();
    }

private:

    /// Helper for constructor.
    void init();

    ///Generate 2 poll directions on a unit n-sphere
    /**
     - A single direction on unit n-sphere is computed (Poll::computeDirOnUnitSphere).
     - The negative direction is added.
     \param directions  The directions obtained for this poll -- \b OUT.
     \param n                      The dimension of the variable space -- \b IN.
      */
     void generateUnitPollDirections(std::list<Direction> &directions, const size_t n) const override final;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DOUBLEPOLLMETHOD__
