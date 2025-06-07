#ifndef __NOMAD_4_5_DMULTIMADSNMSEARCHMETHOD__
#define __NOMAD_4_5_DMULTIMADSNMSEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"
#include "../../Algos/Mads/NMSearchMethod.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform a Search method for DMultiMads using Nelder-Mead simplex algorithm.
/**
 NM works only for single objective. We need to modify the multi objective problem
 into a single objective problem before launching NM. Once NM is done the DMultiMads barrier
 must be updated with the NM points.
 The regular NMSearchMethod is disabled when running DMultiMads.
 */
class DMultiMadsNMSearchMethod final : public NMSearchMethod
{
private:
    size_t _tagBefore; // Use for finding NM trial points in cache
    
    std::shared_ptr<BarrierBase> _ref_dmads_barrier;
    NOMAD::ComputeType _ref_compute_type;

    bool _use_dom_strategy;
    
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit DMultiMadsNMSearchMethod(const Step* parentStep)
      : NMSearchMethod(parentStep)
    {
        init();
    }
    
    virtual bool runImp() override;

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
     void generateTrialPointsFinal() override;
    
    // Helpers for runImp
    void preRunValidations();

    // MultiMads strategy
    void prepareSingleObjectiveRun(const NOMAD::ArrayOfDouble& ref);
    void prepareMultiMadsRun(const NOMAD::ArrayOfDouble& ref);
    bool runMultiMadsStrategy();

    // DoM strategy
    bool runDoMStrategy();

    bool postRunUpdates();
    NOMAD::ArrayOfDouble computeReferencePoint(const NOMAD::DMultiMadsBarrier& barrier) const;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DMULTIMADSNMSEARCHMETHOD__

