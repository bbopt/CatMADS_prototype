
#ifndef __NOMAD_4_5_EVCINTERFACE__
#define __NOMAD_4_5_EVCINTERFACE__

#include "../Algos/Step.hpp"
#include "../Eval/EvaluatorControl.hpp"

#include "../nomad_nsbegin.hpp"


/// Class interface with EvaluatorControl, used by an Algorithm step through IterationUtils
/**
 \todo Complete documentation
 */
class EvcInterface
{
private:
    const Step* _step;      ///< Step that uses the EvaluatorControl
    Point _fixedVariable;   ///< Full dimension point including fixed variables

    DLL_ALGO_API static std::shared_ptr<EvaluatorControl> _evaluatorControl; ///< Static EvaluatorControl

public:
    /// Constructor
    /**
     \param step            The step using this EvcInterface
     */
    explicit EvcInterface(const Step* step)
      : _step(step)
    {
        init();
    }

    /*---------*/
    /* Get/Set */
    /*---------*/

    static const std::shared_ptr<EvaluatorControl> getEvaluatorControl()
    {
        return _evaluatorControl;
    }

    /**
    Set the EvaluatorControl to NULL.
    Useful for Runner between two optimization problems to reset all counters. EvaluatorControl is initiated in MainStep::startImp()
     */
    static void resetEvaluatorControl()
    {
        _evaluatorControl.reset();
        NOMAD::EvaluatorControl::resetCallbacks();
    }

    /**
     If the evaluatorControl attribute is NULL, throws an exception.
     */
    static void setEvaluatorControl(const std::shared_ptr<EvaluatorControl>& evaluatorControl);

    /// Interface for EvaluatorControl::setBarrier.
    /**
     Set the barrier. Can be a full space or subspace barrier. Evaluator control cares only of outputs for detecting success.
     */
    void setBarrier(const std::shared_ptr<BarrierBase>& subBarrier);


    /// Get all evaluated points
    /**
     \note              _evaluatedPoints is cleared
     */
    std::vector<EvalPoint> retrieveAllEvaluatedPoints();


    /*---------------*/
    /* Other Methods */
    /*-------------- */

    // This method may be used by MegaIteration, or by a SearchMethod or by Poll
    /**
     *  For each point, look if it is in the cache.
     *  If it is, count a cache hit.
     *  If not, convert it to an EvalQueuePoint and add it to EvaluatorControl's Queue.

     \param trialPoints The trial points -- \b IN.
     \param useMesh     Flag to use mesh or not -- \b IN.
     */
    void keepPointsThatNeedEval(const EvalPointSet &trialPoints, bool useMesh = true);
    
    
    // This method is used by Poll to calculate the number of points to add to reach a target number
    /**
     *  For each point, look if it is in the cache.
     *  If not, count it but DO NOT add it to EvaluatorControl's Queue.

     \param trialPoints The trial points to consider-- \b IN.
     \return number of trial points that would need to be evaluated
     */
    size_t countPointsThatNeedEval(const EvalPointSet &trialPoints);
    
    
    /**
     *  For each point, look if it is in the cache.
     *  If it is, return it.

     \param trialPoints The trial points -- \b IN.
     \return vector of evaluated points from cache.
     */
    std::vector<EvalPoint> retrieveEvaluatedPointsFromCache(const EvalPointSet &trialPoints);
    
    // Possible refactoring to prevent code duplication.
    /**
     *  For each point, if true flag, look if it is in the cache. If DoEval, add to the vector for sorting.
     *  IMPORTANT: The points that are added are not for evaluation, just for sort.

     \param trialPoints The trial points -- \b IN.
     \param forceRandom Flag to force random sort -- \b IN.
     \param flagTrimIfNotDoEval A flag to trim of not (optional) -- \b IN.
     */
    std::vector<EvalPoint> getSortedTrialPoints(const EvalPointSet &trialPoints,
                                                bool forceRandom,
                                                bool flagTrimIfNotDoEval=true);


    /**
     When points are generated and added to queue, we can start evaluation.
     */
    SuccessType startEvaluation();

    /// Evaluate a single point.
    /**
     Useful for X0. \n
     This method will convert a point from subspace to full space
     before calling EvaluatorControl's method of the same name.

     \param evalPoint   The point to evaluate -- \b IN/OUT.
     \param hMax        The max infeasibility for keeping points in barrier -- \b IN.
     \return            \c true if evaluation worked (evalOk), \c false otherwise.
    */
    bool evalSinglePoint(EvalPoint &evalPoint, const Double &hMax = INF);

private:
    /// Helper for constructor
    void init();

    /// Helper for init
    /**
     Utility that throws an exception when not verified.
     */
    void verifyStepNotNull();

    /// Helper for init and setEvaluatorControl
    /**
     Utility that throws an exception when not verified.
     */
    static void verifyEvaluatorControlNotNull();

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_EVCINTERFACE__
