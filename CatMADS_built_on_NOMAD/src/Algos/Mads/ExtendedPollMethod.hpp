#ifndef __NOMAD_4_5_EXTENDEDPOLLMETHOD__
#define __NOMAD_4_5_EXTENDEDPOLLMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/Mads/SearchMethodAlgo.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform a custom extended poll method.
/**
TODO explain custom method
 */
class ExtendedPollMethod : public NOMAD::SearchMethodAlgo
{
public:
    /// Constructor
    /**
     /param parentStep      The parent of this extended poll method step -- \b IN.
     */
    explicit ExtendedPollMethod(std::shared_ptr<Mads> & mads )
      : SearchMethodAlgo( mads.get() )
    {
        init();
    }

    /**
     Execute (start, run, end) of the NM algorithm. Returns a \c true flag if the algorithm found better point.
     */
    virtual bool runImp() override { return false; };

private:

    /// Helper for constructor.
    /**
     Test if the NM search is enabled or not. Set the maximum number of trial points.
     */
    void init();
    
    /// MAYBE NOT NEEDED
    /**
     */
    virtual void generateTrialPointsFinal() override {};

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_EXTENDEDPOLLMETHOD__
