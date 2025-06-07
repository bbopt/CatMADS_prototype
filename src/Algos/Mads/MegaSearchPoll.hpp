#ifndef __NOMAD_4_5_MEGASEARCHPOLL__
#define __NOMAD_4_5_MEGASEARCHPOLL__

#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/Mads/Search.hpp"
#include "../../Algos/Mads/Poll.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for the mega search and poll of MADS
/**
 Calling the start function generates search and poll trial points at the same time before starting evaluation.
 Calling the run function starts the evaluations.
 The postprocessing is performed when calling the end function.
 */
class MegaSearchPoll: public Step, public IterationUtils
{
private:
    std::unique_ptr<Poll> _poll;
    std::unique_ptr<Search> _search;

    
public:
    /// Constructor
    /**
     \param parentStep The parent of this step
     */
    explicit MegaSearchPoll(const Step* parentStep)
      : Step(parentStep),
        IterationUtils(parentStep),
        _poll(nullptr),
        _search(nullptr)
    {
        init();
    }

    // Destructor
    virtual ~MegaSearchPoll()
    {
    }

private:

    
    
    /// Generate the trial points for the search and poll steps.
    /**
     Call MegaSearchPoll::generateTrialPoints.
     */
    virtual void    startImp() override;

    ///Start evaluations
    virtual bool    runImp() override;

    /**
     Call for postprocessing: computation of a new hMax and update of the barrier.
     */
    virtual void    endImp() override ;

    void init();

    /// Generate new points to evaluate
    /**
     The trial points are produced using poll and search. The duplicates are removed and they are merged all together.
     Called by IterationUtils::generateTrialPoints().
     */
    void generateTrialPointsImp() override ;


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_MEGASEARCHPOLL__
