#ifndef __NOMAD_4_5_QUAD_MODEL_SLD_ITERATION__
#define __NOMAD_4_5_QUAD_MODEL_SLD_ITERATION__

#include "../../Algos/Iteration.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSld.hpp"
#include "../../Eval/EvalPoint.hpp"
#include "../../Eval/MeshBase.hpp"

#include "../../nomad_nsbegin.hpp"

///
class QuadModelSldIteration: public Iteration
{
private:

    void init();

    /**
     - The center point of the model.
     - Cache points used to build the model are taken around this point.
     */
    const EvalPointPtr _center;

    /**
     The trial points use to create the radiuses to select the training set when building the model
     */
    const EvalPointSet & _trialPoints;
    
    /**
     The Mads mesh can be available if this is called during a Search method. If not, it is set to \c nullptr. When available, trials points can be projected on it.
     */
    const MeshBasePtr _madsMesh;

    std::shared_ptr<QuadModelSld>     _model;

    bool _useForSortingTrialPoints;
    
public:
    /// Constructor
    /**
     \param parentStep      The parent of this step -- \b IN.
     \param center     The frame center -- \b IN.
     \param k               The iteration number -- \b IN.
     \param madsMesh        Mads Mesh for trial point projection (can be null) -- \b IN.
     \param trialPoints   Trial points used to define the selection box  (can be empty, so box is defined with mesh)  -- \b IN.
     */
    explicit QuadModelSldIteration(const Step *parentStep,
                                const EvalPointPtr center,
                                const size_t k = 0,
                                const MeshBasePtr madsMesh = nullptr,
                                const EvalPointSet & trialPoints = emptyEvalPointSet )
      : Iteration(parentStep, k) ,
        _center(center),
        _madsMesh(madsMesh),
        _useForSortingTrialPoints(false),
        _trialPoints(trialPoints)
    {
        init();
    }


    /// \brief Destructor
    /// When iteration is done, Flush prints output queue.
    virtual ~QuadModelSldIteration()
    {
    }


    /// Access to the quadratic model
    const std::shared_ptr<QuadModelSld> getModel() const { return _model;}
    
    /// Access to the training set
    // const std::shared_ptr<QuadModelSld> getTrainingSet() const { return _trainingSet; }

    /// Access to the frame center (can be undefined)
    const EvalPointPtr getModelCenter() const { return _center ; }

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

#endif // __NOMAD_4_5_QUAD_MODEL_SLD_ITERATION__
