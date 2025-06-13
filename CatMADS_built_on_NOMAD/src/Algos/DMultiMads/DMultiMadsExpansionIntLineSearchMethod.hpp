#ifndef __NOMAD_4_5_DMULTIMADSEXPANSIONINTLINESEARCHMETHOD__
#define __NOMAD_4_5_DMULTIMADSEXPANSIONINTLINESEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/SearchMethodSimple.hpp"
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"

#include "../../nomad_nsbegin.hpp"

/// Expansion integer linesearch method for DMultiMads
/**
 The expansion integer linesearch method is a backtracking linesearch method.
 It tries to expand the current set of solutions by exploring along a direction
 for integer variables only.
 */
class DMultiMadsExpansionIntLineSearchMethod final : public SearchMethodSimple
{
private:
    std::shared_ptr<NOMAD::BarrierBase> _ref_dmads_barrier;

    NOMAD::ArrayOfDouble _lb;
    NOMAD::ArrayOfDouble _ub;
    std::vector<NOMAD::BBInputType> _bbInputTypes;

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit DMultiMadsExpansionIntLineSearchMethod(const NOMAD::Step* parentStep)
      : SearchMethodSimple(parentStep)
    {
        init();
    }

private:

    /// Helper for constructor.
    /**
     Test if the DmultiMads expansion integer linesearch is enabled or not.
     */
    void init();

    void preRunValidations();

    NOMAD::Direction computePrimitiveDirection(const NOMAD::Point& frameCenter,
                                               const NOMAD::Point& pointFrom,
                                               int& initStepSize) const;

    int computeMaxStepSize(const NOMAD::Point& frameCenter,
                           const NOMAD::Direction& dir,
                           const NOMAD::ArrayOfDouble& lb,
                           const NOMAD::ArrayOfDouble& ub) const;

    bool isInBarrier(const NOMAD::Point& x) const;

    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodAlgo::generateTrialPointsFinal 
     The expansion integer linesearch method generates points along a direction
     for integer variables only.
     */
     void generateTrialPointsFinal() override;
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DMULTIMADSEXPANSIONINTLINESEARCHMETHOD__
