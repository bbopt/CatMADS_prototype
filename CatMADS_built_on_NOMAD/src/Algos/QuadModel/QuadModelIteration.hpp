#ifndef __NOMAD_4_5_QUAD_MODEL_ITERATION__
#define __NOMAD_4_5_QUAD_MODEL_ITERATION__

#include "../../Algos/Iteration.hpp"
#include "../../Eval/EvalPoint.hpp"
#include "../../Eval/MeshBase.hpp"
#include "../../../ext/sgtelib/src/Surrogate.hpp"
#include "../../../ext/sgtelib/src/TrainingSet.hpp"

#include "../../nomad_nsbegin.hpp"

///
class QuadModelIteration: public Iteration
{
private:

    void init();


    /**
     - The reference center point
     - Can be null, so trial points are used to define model center.
    */
    const EvalPointPtr _refCenter;

    const Point * _initialPoint;  ///< Initial point for the QP solver. Preferred over _refCenter  if it is defined.

    /**
     The trial points use to create the radius's to select the training set when building the model
     */
    const EvalPointSet & _trialPoints;
    
    /**
     For multi-objective we can combine all objectives into a single objective for the model
     */
    bool _flagPriorCombineObjsForModel;
    
    /**
     The Mads mesh can be available if this is called during a Search method. If not, it is set to \c nullptr. When available, trials points can be projected on it.
     */
    const MeshBasePtr _madsMesh;

    bool _useForSortingTrialPoints;
    
protected:
    std::shared_ptr<SGTELIB::TrainingSet>   _trainingSet; ///<
    std::shared_ptr<SGTELIB::Surrogate>     _model;

    
public:
    /// Constructor
    /**
     \param parentStep      The parent of this step -- \b IN.
     \param center     The frame center (cen be null, so model center is defined with trial points) -- \b IN.
     \param k               The iteration number -- \b IN.
     \param madsMesh        Mads Mesh for trial point projection (can be null) -- \b IN.
     \param trialPoints   Trial points used to define the selection box  (can be empty, so box is defined with mesh)  -- \b IN.
     \param initialPoint   Initial point for the QP solver. Preferred over center if it is defined. -- \b IN.
     */
    explicit QuadModelIteration(const Step *parentStep,
                                const EvalPointPtr center,
                                const size_t k = 0,
                                const MeshBasePtr madsMesh = nullptr,
                                const EvalPointSet & trialPoints = emptyEvalPointSet,
                                bool flagPriorCombineObjsForModel = false,
                                const Point * initialPoint = nullptr)
      : Iteration(parentStep, k) ,
        _refCenter(center),
        _madsMesh(madsMesh),
        _useForSortingTrialPoints(false),
        _trialPoints(trialPoints),
        _initialPoint(initialPoint),
        _flagPriorCombineObjsForModel(flagPriorCombineObjsForModel)
    {
        init();
    }


    /// \brief Destructor
    /// When iteration is done, Flush prints output queue.
    virtual ~QuadModelIteration()
    {
        /// Reset the model and the training set.
        if (nullptr != _model)
        {
            _model.reset();
        }
        
        if (nullptr != _trainingSet)
        {
            _trainingSet.reset();
        }
    }


    /// Access to the quadratic model
    const std::shared_ptr<SGTELIB::Surrogate> getModel() const { return _model;}
    
    /// Access to the training set
    const std::shared_ptr<SGTELIB::TrainingSet> getTrainingSet() const { return _trainingSet; }

    /// Access to the frame center (can be undefined)
    const EvalPointPtr getRefCenter() const { return _refCenter ; }
    
    /// Access to the initial point
    const Point * getInitialPoint() const { return _initialPoint; }

    /// Reimplement to have access to the mesh (can be null)
    const MeshBasePtr getMesh() const override { return _madsMesh; }
    
    /// Reimplement the access to the name. If the class is used for sorting trial points we get name without algo name.
    std::string getName() const override;

protected:

    /// Manage  quad model update
    /**
     Use the cache to determine a quad model.
     */
    virtual void startImp() override;

    virtual bool runImp() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_ITERATION__
