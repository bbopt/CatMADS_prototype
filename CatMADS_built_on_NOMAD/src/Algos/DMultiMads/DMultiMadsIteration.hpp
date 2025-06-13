#ifndef __NOMAD_4_5_DMULTIMADSITERATION__
#define __NOMAD_4_5_DMULTIMADSITERATION__

#include "../../Algos/Iteration.hpp"
#include "../../Algos/DMultiMads/DMultiMadsUpdate.hpp"
#include "../../Algos/Mads/Poll.hpp"
#include "../../Algos/Mads/Search.hpp"
#include "../../Eval/MeshBase.hpp"


#include "../../nomad_nsbegin.hpp"

/// Class for template algo iterations
/**
Iteration manages the update of the center point (best feasible or best infeasible) around which the trial points are generated.
Trial points generation is performed by random sampling. Evaluation is done. Iteration success is passed to mega iteration.
 */
class DMultiMadsIteration: public Iteration
{
private:
    /// Helper for constructor
    void init ();

    /**
     The current frame center of this iteration. This frame center is either the primary or secondary frame center used by poll and search methods.
     The frame center stores a mesh used by poll and search methods.
     Important note: primary and secondary frame centers will use the single mesh associated to the frame center.
     */
    EvalPointPtr _frameCenter;
    
    std::unique_ptr<NOMAD::DMultiMadsUpdate> _DMultiMadsAlgoUpdate;
    
    std::unique_ptr<Poll> _poll;
    std::unique_ptr<Search> _search;
    
    MeshBasePtr  _mesh;        ///< Mesh of the frame center on which the points are generated
    
    SuccessType _previousSuccess ; ///<  Step success is initialized before calling update. This stores the success of previous iteration.
    
public:
    /// Constructor
    /**
     \param parentStep         The parent of this step -- \b IN.
     \param frameCenter        The frame center -- \b IN.
     \param k                  The iteration number -- \b IN.
     */
    explicit DMultiMadsIteration(const Step *parentStep,
                                 const EvalPointPtr frameCenter,
                                 const size_t k,
                                 const MeshBasePtr initialMesh)
      : Iteration(parentStep, k),
        _frameCenter(frameCenter),
        _DMultiMadsAlgoUpdate(nullptr),
        _poll(nullptr),
        _search(nullptr),
        _mesh(initialMesh),
        _previousSuccess(SuccessType::UNDEFINED)
    {
        init();
    }


    // Get/Set
    const EvalPointPtr getFrameCenter() const { return _frameCenter ; }
    void setFrameCenter(EvalPointPtr frameCenter) { _frameCenter = frameCenter ;}
    
    /**
     The DMultiMads algorithm iteration possesses a mesh. This mesh changes during iterations. For the first iteration, a mesh obtained during algo initialization is used. After that, it is the frame center mesh. It is updated during start.
     \remark Used by Step::getIterationMesh() to pass the mesh whenever needed
     */
    const MeshBasePtr getMesh() const override { return _mesh; }
    
    SuccessType getPreviousSuccessType() const { return _previousSuccess ; }
    
protected:

    /// Implementation of run task.
    /**
     Evaluate trial point(s).
     Set the stop reason and also updates the Nelder Mead mega iteration success.
     */
    virtual bool runImp() override ;

    /// Implementation of start task.
    /**
     Update the barrier and create the trial point randomly from the best incumbent.
     */
    virtual void startImp() override;


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DMULTIMADSITERATION__
