
#ifndef __NOMAD_4_5_SIMPLELINESEARCHMETHOD__
#define __NOMAD_4_5_SIMPLELINESEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/SearchMethodAlgo.hpp"
#include "../../Algos/SimpleLineSearch/SimpleLineSearch.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform a simple line search method from a speculative search.
/**
 Perform a speculative search plus an extra point.
 If speculative point is not a success, find the best position along the direction of last success.
  */
class SimpleLineSearchMethod final : public SearchMethodAlgo
{
private:
    
    std::shared_ptr<AlgoStopReasons<SimpleLineSearchStopType>> _simpleLineSearchStopReasons;
    
    std::unique_ptr<SimpleLineSearch> _simpleLineSearch;

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit SimpleLineSearchMethod(const Step* parentStep )
      : SearchMethodAlgo(parentStep ),
        _simpleLineSearchStopReasons(nullptr),
        _simpleLineSearch(nullptr)
    {
        init();
    }


    /**
     Execute (start, run, end) of the simple line search. Returns a \c true flag if the algorithm found better point.
     */
    virtual bool runImp() override ;


private:

    /// Helper for constructor.
    /**
     Test if search is enabled or not. Set the maximum number of trial points.
     */
    void init();

    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodAlgo::generateTrialPointsFinal 
     Iterative random generation of trial points
     */
     void generateTrialPointsFinal() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SIMPLELINESEARCHMETHOD__

