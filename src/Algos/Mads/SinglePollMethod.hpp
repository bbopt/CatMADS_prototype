#ifndef __NOMAD_4_5_SINGLEPOLLMETHOD__
#define __NOMAD_4_5_SINGLEPOLLMETHOD__

#include "../../Algos/Mads/PollMethodBase.hpp"
#include "../../nomad_nsbegin.hpp"

/// Class to perform single poll directions.
class SinglePollMethod final : public PollMethodBase
{
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit SinglePollMethod(const Step* parentStep,
                              const EvalPointPtr frameCenter)
      : PollMethodBase(parentStep, frameCenter)
    {
        init();
    }

private:

    /// Helper for constructor.
    void init();

    ///Generate a single poll direction on a unit N-sphere (no evaluation)
    /**
     - A single direction on unit n-sphere is computed (Poll::computeDirOnUnitSphere).
     \param directions  The direction obtained for this poll -- \b OUT.
     \param n                      The dimension of the variable space -- \b IN.
      */
     void generateUnitPollDirections(std::list<Direction> &directions, const size_t n) const override final;
    
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SINGLEPOLLMETHOD__
