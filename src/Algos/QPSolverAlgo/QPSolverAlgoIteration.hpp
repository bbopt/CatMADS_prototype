#ifndef __NOMAD_4_5_QPSOLVERALGOITERATION__
#define __NOMAD_4_5_QPSOLVERALGOITERATION__

#include "../../Algos/Iteration.hpp"
#include "../../Eval/EvalPoint.hpp"
#include "../../Eval/MeshBase.hpp"
#include "../../../ext/sgtelib/src/Surrogate.hpp"
#include "../../../ext/sgtelib/src/TrainingSet.hpp"

#include "../../Algos/QuadModel/QuadModelUpdate.hpp"
#include "../../Algos/QuadModel/QuadModelIteration.hpp"


#include "../../nomad_nsbegin.hpp"

/// Class for QPSolver global iterations
/**
Iteration manages the update of the center point (best feasible or best infeasible) around which the trial points are generated.
Trial points generation is performed by QPsolver method. Evaluation is done. Iteration success is passed to mega iteration.
 */
class QPSolverAlgoIteration: public QuadModelIteration
{

    
public:
    /// Constructor
    /**
     \param parentStep         The parent of this step -- \b IN.
     \param frameCenter        The frame center -- \b IN.
     \param k                  The iteration number -- \b IN.
     \param madsMesh        Mads Mesh for trial point projection (can be null) -- \b IN.
     \param trialPoints   Trial points used to define the selection box  (can be empty, so box is defined with mesh)  -- \b IN.
     */
    explicit QPSolverAlgoIteration(const Step *parentStep,
                                   const EvalPointPtr frameCenter,
                                   const size_t k = 0,
                                   const MeshBasePtr madsMesh = nullptr,
                                   const EvalPointSet & trialPoints = emptyEvalPointSet)
      : QuadModelIteration(parentStep, frameCenter,k, madsMesh, trialPoints)
    {
    }


//    // Get/Set
//    const EvalPointPtr getFrameCenter() const { return _frameCenter ; }
//    void setFrameCenter(EvalPointPtr frameCenter) { _frameCenter = frameCenter ;}
//
//    /// Access to the quadratic model
//    const std::shared_ptr<SGTELIB::Surrogate> getModel() const { return _model;}
//
//    /// Access to the training set
//    const std::shared_ptr<SGTELIB::TrainingSet> getTrainingSet() const { return _trainingSet; }
//
//    /// Reimplement to have access to the mesh (can be null)
//    const MeshBasePtr getMesh() const override { return _madsMesh; }
//
//    // Reimplement the access to the name. If the class is used for sorting trial points we get name without algo name.
//    //std::string getName() const override;

protected:

    /// Implementation of run task.
    /**
     Evaluate trial point(s).
     Set the stop reason and also updates the Nelder Mead mega iteration success.
     */
    virtual bool runImp() override ;



};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QPSOLVERALGOITERATION__
