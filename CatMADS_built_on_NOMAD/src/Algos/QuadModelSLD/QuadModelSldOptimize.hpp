#ifndef __NOMAD_4_5_QUAD_MODEL_SLD_OPTIMIZE__
#define __NOMAD_4_5_QUAD_MODEL_SLD_OPTIMIZE__

#include "../../Algos/Step.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldIterationUtils.hpp"

#include "../../nomad_nsbegin.hpp"


/// Class to create trial points by performing quadratic model optimization using Mads
/// This class uses Sébastien Le Digabel implementation of Quad models as in Nomad 3
/// TODO: Sgtelib quad model does the same job. This implementation will most likely be removed in the future.
/**
 - Start, run and end tasks are performed.
 - Start: the quadratic model optimization problem is setup and solved by calling startImp. Call ::generateTrialPoints.
 - Run: trial (oracle) points are evaluated with EvalType::BB. Set the stop reason.
 - End: Remove from cache EvalType::MODEL only cache points.
 */
class QuadModelSldOptimize : public Step, public QuadModelSldIterationUtils
{
private:

    OutputLevel                         _displayLevel;
    const std::shared_ptr<PbParameters> _refPbParams; ///< Reference to the original problem parameters.

    std::shared_ptr<RunParameters>      _optRunParams; ///< run parameters for model optimization
    std::shared_ptr<PbParameters>       _optPbParams; ///< pb parameters for model optimization


    
public:
    /// Constructor
    /* Parent must explicitely be a (pointer to a) QuadModelAlgo.
     * Run parameters will be recomputed for model optimization.
     */
    explicit QuadModelSldOptimize(const Step* parentStep,
                               const std::shared_ptr<PbParameters>               refPbParams)
      : Step(parentStep),
      QuadModelSldIterationUtils (parentStep),
        _displayLevel(OutputLevel::LEVEL_INFO),
        _refPbParams(refPbParams),
        _optRunParams(nullptr),
        _optPbParams(nullptr)
    {
        init();
    }

    /// Generate new points to evaluate
    /**
     - Setup the evaluator control parameters.
     - Manage display of sub-optimization.
     - Setup evaluator (EvalType::MODEL) and success type identification function.
     - Setup the bounds and fixed variables from the trainingSet of the quadratic model.
     - Setup run and pb parameters for Mads
     - Perform start, run and end tasks on Mads.
     - best feasible and best infeasible (if available) are inserted as trial points.
     */
    void generateTrialPointsImp() override;
        
    
private:
    void init();

    virtual void startImp() override; ///< The quadratic model optimization problem is setup and solved by calling startImp. Calls ::generateTrialPoints.
    virtual bool runImp() override; ///< Trial (oracle) points are evaluated with EvalType::BB. Set the stop reason.
    virtual void endImp() override; ///< Remove from cache EvalType::MODEL only cache points.

    // Helpers
    void setupRunParameters();
    void setupPbParameters();

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_SLD_OPTIMIZE__
