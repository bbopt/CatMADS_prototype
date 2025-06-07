#ifndef __NOMAD_4_5_TEMPLATEALGORANDOM__
#define __NOMAD_4_5_TEMPLATEALGORANDOM__

#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/Step.hpp"
#include "../../Eval/EvalPoint.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform random generation of trial points.
/**
  
 */
class TemplateAlgoRandom: public Step, public IterationUtils
{
private:
    
    NOMAD::EvalPointPtr _center; // The point around which we sample randomly
    
    NOMAD::ArrayOfDouble _boxSize; // The sample is made within boxes of size {1...k}*Delta
    
    
public:
    /// Constructor
    /**
     \param parentStep The parent of this step
     */
    explicit TemplateAlgoRandom(const Step* parentStep)
      : Step(parentStep),
        IterationUtils(parentStep),
        _center(nullptr)
    {
        init();
    }
    virtual ~TemplateAlgoRandom() {}

    /// Generate new points to evaluate
    /**
     A new point is obtained using the simplex. xt = yc + delta*d. Delta is the multiplicative factor that can increase or decrease with success. \n

     The point is snapped to bounds and projected on the mesh.
     */
    void generateTrialPointsImp() override;


private:

    /**
     The delta parameter used to create the trial point is different. The possible delta parameters are obtained from _runParams. The validity of the parameters are checked. \n
     The flag to perform a standalone template random algo optimization is also set.
     */
    void init();

    /// Implementation of the start task.
    /**
     Call TemplateAlgoRandom::generateTrialPoints and update the trial points.
     */
    virtual void    startImp() override ;

    /// Implementation of the run task.
    /**
     Evaluate the trial point and store it locally. Call IterationUtils::postProcessing.

     \return \c true if a better point is obtained \c false otherwise.
     */
    virtual bool    runImp() override ;

    /// No end task is required
    virtual void    endImp() override {}

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_TEMPLATEALGORANDOM__
