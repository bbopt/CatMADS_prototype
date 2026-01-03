#ifndef __NOMAD_4_5_QUAD_MODEL_SINGLE_PASS__
#define __NOMAD_4_5_QUAD_MODEL_SINGLE_PASS__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/QuadModel/QuadModelIteration.hpp"
#include "../../Algos/QuadModel/QuadModelIterationUtils.hpp"

#include "../../nomad_nsbegin.hpp"

/**
 Final class to generate points for single pass on a given frame center. Used as Search method and for n+1 th direction of Ortho Mads n+1 poll.
 The start, run and end tasks are empty. No evaluations are performed.
 The QuadModelSinglePass::generateTrialPoints function manages the creation process. The sample set to build the quad model is created by calling QuadModelIteration::startImp(). The points are not projected on mesh (done in SearchMethodBase).
 */
class QuadModelSinglePass final: public QuadModelIteration, public QuadModelIterationUtils
{
private:
    
    bool _flagUseScaledModel;   ///< The model can be scaled between [0,1]^n (rotation facilitates setting the bounds for optimization)
    
    bool _flagPriorCombineObjsForModel;
    
    const std::vector<Direction> & _scalingDirections;
    

public:
    /// Constructor
    /**
     \param parentStep                  The parent step of this step -- \b IN.
     \param frameCenter                The "center" around which to construct the training set  -- \b IN.
     \param madsMesh                       The Mads mesh for constructing the training set (no snap on mesh is performed) -- \b IN.
     \param scalingDirections   The directions used for scaling and bounding the model (optional) -- \b IN.
     */
    explicit QuadModelSinglePass(const Step* parentStep,
                                 const EvalPointPtr frameCenter,
                                 const MeshBasePtr madsMesh,
                                 const std::vector<Direction> & scalingDirections,
                                 bool flagPriorCombineObjsForModel = false,
                                 const Point * initialPoint = nullptr)
      : QuadModelIteration(parentStep, frameCenter, 0, madsMesh, {} /* no trial points */, flagPriorCombineObjsForModel, initialPoint),
        QuadModelIterationUtils(parentStep),
        _scalingDirections(scalingDirections),
        _flagPriorCombineObjsForModel(flagPriorCombineObjsForModel)
    {
        _stopReasons = std::make_shared<AlgoStopReasons<ModelStopType>>();
        _flagUseScaledModel = (_scalingDirections.size() > 0);
        
    }
    // No Destructor needed - keep defaults.

    /// Implementation of start task. Nothing to do.
    void startImp() override {}

    /// Implementation of run task. Nothing to do.
    bool runImp() override { return  false;}

    /// Implementation of run task. Nothing to do.
    void endImp() override {}

    /// Generate trial points
    /**
     - Update the quadratic model.
     - Optimize the quadratic model problem.
     - Insert the best feasible and best infeasible (if available) as trial points.
     */
    void generateTrialPointsImp() override;

};


#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_SINGLE_PASS__
