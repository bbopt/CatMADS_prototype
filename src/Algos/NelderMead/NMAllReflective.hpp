#ifndef __NOMAD_4_5_NMALLREFLECTIVE__
#define __NOMAD_4_5_NMALLREFLECTIVE__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/NelderMead/NMIteration.hpp"
#include "../../Algos/NelderMead/NMIterationUtils.hpp"

#include "../../nomad_nsbegin.hpp"

/**
 Class to generate points for single pass NM on all reflective steps (REFLECT, EXPAND, INSIDE_CONTRACTION and OUTSIDE_CONTRACTION).
 The NMAllReflective::startImp function manages the creation process. The initial simplex is created by calling NMIteration::startImp(). The points are projected on mesh and updated with the information of the creating simplex center.
 */
class NMAllReflective: public NMIteration, public NMIterationUtils
{
public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param simplexCenter   The MADS frame center is used as simplex "center"  -- \b IN.
     \param madsMesh        Mads Mesh for trial point projection and simplex scaling (can be null) -- \b IN.
     */
    explicit NMAllReflective(const Step* parentStep,
                             const EvalPointPtr simplexCenter,
                             const MeshBasePtr& madsMesh)
      : NMIteration(parentStep, simplexCenter, 0, madsMesh),
        NMIterationUtils(parentStep)
    {
        _stopReasons = std::make_shared<AlgoStopReasons<NMStopType>>();
    }
    // No Destructor needed - keep defaults.


private:

    /// Implementation of start tasks.
    /**
     - call the default Iteration::startImp
     - create the initial simplex if it is empty.
     - call NMAllReflective::generateTrialPoints
     - verify that trial points are on mesh.
     - update points with simplex center.
     */
    void startImp() override ;

    /// Implementation of run task. Nothing to do.
    bool runImp() override { return  false;}

    /// Implementation of run task. Nothing to do.
    void endImp() override {}

    void generateTrialPointsImp() override;

protected:
    // Override default Step::getName that seeks the algo name. Here we have a single Iteration and the class does not derive from an Algo.
    std::string getName() const override;
    
};


#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_NMALLREFLECTIVE__
