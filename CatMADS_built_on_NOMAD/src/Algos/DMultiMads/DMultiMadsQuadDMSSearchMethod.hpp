#ifndef __NOMAD_4_5_DMULTIMADSQUADDMSSEARCHMETHOD__
#define __NOMAD_4_5_DMULTIMADSQUADDMSSEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"
#include "../../Algos/Mads/SearchMethodAlgo.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform a Search method for DMultiMads using Quad DMS algorithm.
/**
 The Quad DMS algorithm solves a succession of single-objective optimization
 subproblems. It first tries to minimize each objective component separately.
 If the set of current Pareto solutions does not change, it keeps on going
 with the minimization of the maximum of two objective components, unless it
 modifies the current set of Pareto solutions or it has reached the maximum
 number of objectives (min max fi(x)).
 In the worst case, 2^m - 1 problems will be solved, where m is the number
 of objectives of the original multiobjective problem, which can be costly
 when the number of objectives is high.
 */
class DMultiMadsQuadDMSSearchMethod final : public SearchMethodAlgo
{
private:
    
    std::shared_ptr<DMultiMadsBarrier> _ref_dmads_barrier;
    std::vector<NOMAD::EvalPointPtr> _paretoElements; ///< Current Pareto set of solutions

    ComputeType _ref_compute_type;
    
    std::list<size_t> _activeObjsIndex;
    size_t _l; // _l+1 is the current number of active objectives for combination (combination of two objs (_l=1), combination of three obj (_l=2)).
    size_t _m; // Number of objectives
    std::vector<size_t> _indices;
    
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit DMultiMadsQuadDMSSearchMethod(const Step* parentStep)
      : SearchMethodAlgo(parentStep),
        _m(0),
        _l(0)
    {
        init();
    }
    
    virtual bool runImp() override;
    
    // Temp for testing obj combinations
    void testObjCombinations();

private:

    /// Helper for constructor.
    /**
     Test if the Quad DMS search is enabled or not. Set the maximum number of trial points.
     */
    void init();

    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodAlgo::generateTrialPointsFinal 
     */
     void generateTrialPointsFinal() override;
    
    // Helpers for runImp
    void preRun();
    void prepareSingleObjectiveRun();
    bool postRunUpdates();
    bool selectObjCombination() ;
    void generateTrialPointOnSingleObjCombination();
    bool changeLevelAndUpdateIndex();

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DMULTIMADSQUADDMSSEARCHMETHOD__
