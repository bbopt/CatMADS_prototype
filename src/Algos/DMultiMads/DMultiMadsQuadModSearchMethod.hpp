#ifndef __NOMAD_4_5_DMULTIMADSQUADMODSEARCHMETHOD__
#define __NOMAD_4_5_DMULTIMADSQUADMODSEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"
#include "../../Algos/Mads/SearchMethodSimple.hpp"



#include "../../nomad_nsbegin.hpp"

/// Class to perform a Search method for DMultiMads using Quad model optimization algorithm.
/**
 Quadratic model optimization works only for single objective. We need to modify the multi objective problem
 into a single objective problem before launching quadratic model optimization.
 Once quad model optim is done the DMultiMads barrier
 must be updated with the new points.
 The regular QuadModelSearchMethod is disabled when running DMultiMads.
 */
class DMultiMadsQuadModSearchMethod final : public SearchMethodSimple
{
private:
    
    ComputeType _ref_compute_type;
    
    bool _flagPriorCombineObjsForModel;

    bool _use_dom_strategy;
    
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit DMultiMadsQuadModSearchMethod(const Step* parentStep)
      : SearchMethodSimple(parentStep)
    {
        init();
    }

private:

    /// Helper for constructor.
    /**
     Test if the quad model search is enabled or not. 
     */
    void init();

    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodAlgo::generateTrialPointsFinal 
     
     Perform one quad model optimization to produce trial points.
     */
     virtual void generateTrialPointsFinal() override;
    
    // Helpers for generateTrialPoints

    // MultiMads strategy
    void prepareSingleObjectiveRun(const NOMAD::ArrayOfDouble& ref);
    void prepareMultiMadsRun(const NOMAD::ArrayOfDouble& ref);
    void runMultiMadsStrategy();

    // DoM strategy
    void runDoMStrategy();

    NOMAD::ArrayOfDouble computeReferencePoint(const NOMAD::DMultiMadsBarrier& barrier) const;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DMULTIMADSQUADMODSEARCHMETHOD__

