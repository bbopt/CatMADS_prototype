#ifndef __NOMAD_4_5_DMULTIMADSMIDDLEPOINTSEARCHMETHOD__
#define __NOMAD_4_5_DMULTIMADSMIDDLEPOINTSEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/SearchMethodSimple.hpp"
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"

#include "../../nomad_nsbegin.hpp"

/// Middle point search method for DMultiMads
/**
 The middle point search method consists in trying to fill gaps in the set of current solutions
 by generating candidates in the (potentially) largest zones in the objective space.
 */
class DMultiMadsMiddlePointSearchMethod final : public SearchMethodSimple
{
private:
    std::shared_ptr<NOMAD::BarrierBase> _ref_dmads_barrier;

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit DMultiMadsMiddlePointSearchMethod(const NOMAD::Step* parentStep)
      : SearchMethodSimple(parentStep)
    {
        init();
    }

private:

    /// Helper for constructor.
    /**
     Test if the DmultiMads Middle Point Search is enabled or not. Set the maximum number of tentatives to propose
     points.
     */
    void init();

    /// Helper for generateTrialPoints
    void preRunValidations();

    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodAlgo::generateTrialPointsFinal 
     The middle point search generates points to fill gaps in the current set of solutions.
     */
     void generateTrialPointsFinal() override;
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DMULTIMADSMIDDLEPOINTSEARCHMETHOD__
