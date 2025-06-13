#ifndef __NOMAD_4_5_TEMPLATEALGOITERATION__
#define __NOMAD_4_5_TEMPLATEALGOITERATION__

#include "../../Algos/Iteration.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoRandom.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoUpdate.hpp"
#include "../../Eval/MeshBase.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for template algo iterations
/**
Iteration manages the update of the center point (best feasible or best infeasible) around which the trial points are generated.
Trial points generation is performed by random sampling. Evaluation is done. Iteration success is passed to mega iteration.
 */
class TemplateAlgoIteration: public Iteration
{
private:
    /// Helper for constructor
    void init ();

    /**
     The current frame center of this iteration.
     This is the frame center of MADS when used for a search method of MADS.
     */
    EvalPointPtr _frameCenter;
    
    std::unique_ptr<NOMAD::TemplateAlgoUpdate> _templateAlgoUpdate;
    
protected:
    std::unique_ptr<NOMAD::TemplateAlgoRandom> _templateAlgoRandom;
    
public:
    /// Constructor
    /**
     \param parentStep         The parent of this step -- \b IN.
     \param frameCenter        The frame center -- \b IN.
     \param k                  The iteration number -- \b IN.
     */
    explicit TemplateAlgoIteration(const Step *parentStep,
                                   const EvalPointPtr frameCenter,
                                   const size_t k)
      : Iteration(parentStep, k),
        _frameCenter(frameCenter)
    {
        init();
    }


    // Get/Set
    const EvalPointPtr getFrameCenter() const { return _frameCenter ; }
    void setFrameCenter(EvalPointPtr frameCenter) { _frameCenter = frameCenter ;}
    

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

#endif // __NOMAD_4_5_TEMPLATEALGOITERATION__
