#ifndef __NOMAD_4_5_TEMPLATEALGOSINGLEPASS__
#define __NOMAD_4_5_TEMPLATEALGOSINGLEPASS__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoIteration.hpp"
#include "../../Algos/IterationUtils.hpp"

#include "../../nomad_nsbegin.hpp"

/**
 Class to generate points for single pass of random generation (no iteration).
 The points are projected on mesh.
 */
class TemplateAlgoSinglePass: public TemplateAlgoIteration, public IterationUtils
{
public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param center             The frame center around which is done the random sampling  -- \b IN.
     */
    explicit TemplateAlgoSinglePass(const Step* parentStep,
                                    const EvalPointPtr center)
      : TemplateAlgoIteration(parentStep, center, 0 ),
        IterationUtils(parentStep)
    {
        init();
    }
    
    // No special destructor needed - keep defaults.


private:

    /// Implementation of start tasks.
    /**
     - call the default Iteration::startImp
     - create the initial simplex if it is empty.
     - generate trial points using
     - verify that trial points are on mesh.
     */
    void startImp() override ;

    /// Implementation of run task. Nothing to do.
    bool runImp() override { return  false;}

    /// Implementation of run task. Nothing to do.
    void endImp() override {}

    /// Generation of trial points uses the random generator
    void generateTrialPointsImp() override;
    
    void init();

protected:
    // Override default Step::getName that seeks the algo name. Here we have a single Iteration and the class does not derive from an Algo.
    std::string getName() const override;
    
};


#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_TEMPLATEALGOSINGLEPASS__
