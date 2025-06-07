#ifndef __NOMAD_4_5_QPSOLVERALGOSINGLEPASS__
#define __NOMAD_4_5_QPSOLVERALGOSINGLEPASS__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/QPSolverAlgo/QPSolverAlgoIteration.hpp"
#include "../../Algos/QuadModel/QuadModelIteration.hpp"
#include "../../Algos/QuadModel/QuadModelIterationUtils.hpp"
#include "../../Algos/IterationUtils.hpp"

#include "../../nomad_nsbegin.hpp"

/**
 Class to generate points with a single pass of QP solver generation (no BB evaluation is performed).
 The points are projected on mesh in the search method.
 */
class QPSolverAlgoSinglePass: public QuadModelIteration, public QuadModelIterationUtils
{
private:
    
    bool _flagUseScaledModel;   ///< The model can be scaled between [0,1]^n (rotation facilitates setting the bounds for optimization)
    
    bool _flagReducedIterations; ///< Flag to reduce the number of iterations for the QP solver
    
    const std::vector<Direction> & _scalingDirections;
    
    std::shared_ptr<AlgoStopReasons<QPStopType>> _qpStopReason;
        
public:
    /// Constructor
    /**
     \param parentStep                  The parent step of this step -- \b IN.
     \param frameCenter                The "center" around which to construct the training set  -- \b IN.
     \param madsMesh                       The Mads mesh for constructing the training set (no snap on mesh is performed) -- \b IN.
     \param scalingDirections   The directions used for scaling and bounding the model (optional) -- \b IN.
     \param initialPoint              The initial point for the QP solver -- \b IN.
     \param flagReducedIterations  Flag to reduce the number of iterations for the QP solver -- \b IN.
     */
    explicit QPSolverAlgoSinglePass(const Step* parentStep,
                                 const EvalPointPtr frameCenter,
                                 const MeshBasePtr madsMesh,
                                 const std::vector<Direction> & scalingDirections,
                                 const Point * initialPoint = nullptr,
                                 bool flagReducedIterations = false )
      : QuadModelIteration(parentStep, frameCenter, 0, madsMesh, {} /* no trial points */, false /* default flag */ , initialPoint),
        QuadModelIterationUtils(parentStep),
        _scalingDirections(scalingDirections),
        _flagReducedIterations(flagReducedIterations)
    {
        _stopReasons = std::make_shared<AlgoStopReasons<ModelStopType>>();
        _qpStopReason = std::make_shared<AlgoStopReasons<QPStopType>>();
        
        _flagUseScaledModel = (_scalingDirections.size() > 0);
        
    }
    
    std::shared_ptr<AlgoStopReasons<QPStopType>> getQPStopReason() const { return _qpStopReason;}
    
    // No special destructor needed - keep defaults.

private:

    /// Implementation of start tasks.
    /**
     - call the default Iteration::startImp
     - create the model.
     - generate trial points using the model
     - verify that trial points are on mesh.
     */
    void startImp() override {}

    /// Implementation of run task. Nothing to do.
    bool runImp() override { return  false;}

    /// Implementation of run task. Nothing to do.
    void endImp() override {}

    /// Generation of trial points uses the random generator
    void generateTrialPointsImp() override;
    

    
};


#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QPSOLVERALGOSINGLEPASS__
