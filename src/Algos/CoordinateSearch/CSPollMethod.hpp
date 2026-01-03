
#ifndef __NOMAD_4_5_CSPOLLMETHOD__
#define __NOMAD_4_5_CSPOLLMETHOD__

#include "../../Algos/Mads/PollMethodBase.hpp"

#include "../../nomad_nsbegin.hpp"

/**
 Class to perform CS Poll: generate poll directions, create the trial points and perform evaluations.
  The CS poll directions consists of unitary direction of each coordinate separately, that is north, south, east and west directions in 2D.
 Only the generation of poll directions is implemented in this class, the remaining tasks are performed by derived classes.
*/
class CSPollMethod final : public PollMethodBase
{
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit CSPollMethod(const Step* parentStep,
                          const EvalPointPtr frameCenter)
      : PollMethodBase(parentStep, frameCenter)
    {
        init();
    }

private:

    /// Helper for constructor.
    /**

     */
    void init();

   
    /**
     ADD DESCRIPTION
     \param directions  The directions obtained for this poll -- \b OUT.
     \param n                      The dimension of the variable space  -- \b IN.
      */
     void generateUnitPollDirections(std::list<Direction> &directions, const size_t n) const override final;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_CSPOLLMETHOD__
