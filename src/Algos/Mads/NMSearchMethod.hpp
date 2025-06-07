#ifndef __NOMAD_4_5_NMSEARCHMETHOD__
#define __NOMAD_4_5_NMSEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/NelderMead/NM.hpp"
#include "../../Algos/Mads/SearchMethodAlgo.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform a Search method using Nelder Mead simplex algorithm.
/**
 If Nelder Mead search is enabled (check is done in NMSearchMethod::init), the NMSearchMethod::run function manages the execution (start, run, end) of the NM algorithm. \n
 The new trial points can be generated during a single pass of all Nelder Mead reflective steps (REFLECT, EXPAND, INSIDE_CONTRACTION, OUTSIDE_CONTRACTION) ( generateTrialPoint ) or as a NM optimization.
 */
class NMSearchMethod : public SearchMethodAlgo
{
protected:
    std::shared_ptr<AlgoStopReasons<NMStopType>> _nmStopReasons;
    
    std::unique_ptr<NM> _nm;

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit NMSearchMethod(const Step* parentStep )
      : SearchMethodAlgo(parentStep ),
        _nmStopReasons(nullptr),
        _nm(nullptr)
    {
        init();
    }

    /**
     Execute (start, run, end) of the NM algorithm. Returns a \c true flag if the algorithm found better point.
     */
    virtual bool runImp() override ;

private:

    /// Helper for constructor.
    /**
     Test if the NM search is enabled or not. Set the maximum number of trial points.
     */
    void init();
    
    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodAlgo::generateTrialPointsFinal 
     
     Perform one iteration of all reflective steps (Reflect, Expansion, Inside and Outside Contraction). This is just portion of the NM algorithm without iteration.
     */
     virtual void generateTrialPointsFinal() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_NMSEARCHMETHOD__

