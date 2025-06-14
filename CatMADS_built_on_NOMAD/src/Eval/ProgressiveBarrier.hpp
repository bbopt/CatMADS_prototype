#ifndef __NOMAD_4_5_PROGRESSIVEBARRIER__
#define __NOMAD_4_5_PROGRESSIVEBARRIER__

#include "../Eval/BarrierBase.hpp"
#include "../Eval/EvalPoint.hpp"

#include "../nomad_nsbegin.hpp"

/// Class for single objective progressive barrier (for Mads) following algorithm 12.2 of DFBO.
class DLL_EVAL_API ProgressiveBarrier : public BarrierBase
{
private:
    bool _incumbentsAndHMaxUpToDate;
    
public:
    /// Constructor
    /**
     * hMax will be updated during optimization.
     \param hMax            The max of h to keep a point in the barrier -- \b IN.
     \param fixedVariable   The fixed variables have a fixed value -- \b IN.
     \param evalType        Type of evaluation (BB or MODEL) -- \b IN.
     \param computeType  Type of function computation (standard, phase-one or user) -- \b IN.
     \param evalPointList   Additional points to consider in building the barrier -- \b IN.
     \param barrierInitializedFromCache Flag to initialize the barrier from cache -- \b IN.
     */
    ProgressiveBarrier(const Double& hMax = INF,
            const Point& fixedVariable = Point(),
            EvalType evalType = EvalType::BB,
            FHComputeTypeS computeType = defaultFHComputeTypeS,
            const std::vector<EvalPoint>& evalPointList = std::vector<EvalPoint>(),
            bool barrierInitializedFromCache= true)
      : BarrierBase(evalType, computeType, hMax),
        _incumbentsAndHMaxUpToDate(false)
    {
        init(fixedVariable, barrierInitializedFromCache);
        init(fixedVariable,evalPointList);
    }
    
    
    
    // Copy constructor
    ProgressiveBarrier(const ProgressiveBarrier & b) : NOMAD::BarrierBase(b)
    {
        // Do not copy the barrier points. Do not initialize from cache.
    }
    
    std::shared_ptr<BarrierBase> clone() const override {
      return std::make_shared<ProgressiveBarrier>(*this);
    }
    
    /*-----------------*/
    /* Feasible points */
    /*-----------------*/

    /// Update ref best feasible and ref best infeasible values.
    void updateRefBests() override;


    /*---------------*/
    /* Other methods */
    /*---------------*/

    /// Set the hMax of the barrier
    /**
     \param hMax    The hMax -- \b IN.
    */
    void setHMax(const Double &hMax) override;

    ///  xFeas and xInf according to given points.  // TODO : comment of this function does not match declaration
    /* \param evalPointList vector of EvalPoints  -- \b IN.  
     * \param keepAllPoints keep all good points, or keep just one point as in NOMAD 3 -- \b IN.
     * \return true if the Barrier was updated, false otherwise
     * \note Input EvalPoints are already in subproblem dimension
     */
    SuccessType getSuccessTypeOfPoints(const EvalPointPtr xFeas,
                                       const EvalPointPtr xInf) override;

    /// Update xFeas and xInf according to given points.
    /* \param evalPointList vector of EvalPoints  -- \b IN.
     * \param keepAllPoints TODO see if it is needed \b IN.
     * \return true if the barrier feasible and/or infeasible incumbents are changed, false otherwise
     * \note Input EvalPoints are already in subproblem dimension
     */
    bool updateWithPoints(const std::vector<EvalPoint>& evalPointList,
                          const bool keepAllPoints = false,
                          const bool updateInfeasibleIncumbentAndHmax = false ) override;
    
    
    
    EvalPointPtr getCurrentIncumbentInf() const override;
    EvalPointPtr getCurrentIncumbentFeas() const override;

    /// Return the barrier as a string.
    /* May be used for information, or for saving a barrier. In the former case,
     * it may be useful to set parameter max to a small value (e.g., 4). In the
     * latter case, INF_SIZE_T should be used so that all points are saved.
     * \param max Maximum number of feasible and infeasible points to display
     * \return A string describing the barrier
     */
    std::vector<std::string> display(const size_t max = INF_SIZE_T) const override;

private:

    /**
     * \brief Helper function for constructor.
     *
     * Will throw exceptions or output error messages if something is wrong. Will remain silent otherwise.
     \param fixedVariable   The fixed variables have a fixed value     -- \b IN.
     \param barrierInitializedFromCache  Flag to initialize barrier from cache or not. -- \b IN.
     */
    void init(const Point& fixedVariable,
              bool barrierInitializedFromCache) override;

    /**
     * \brief Helper function for constructor.
     *
     * Will throw exceptions or output error messages if something is wrong. Will remain silent otherwise.
     \param fixedVariable   The fixed variables have a fixed value     -- \b IN.
     \param evalPointList   Additional points to consider to construct barrier. -- \b IN.
     */
    void init(const Point& fixedVariable,
              const std::vector<EvalPoint>& evalPointList);

    
    /** Helper for updateWithPoints
     *
     */
    bool dominates(const NOMAD::ArrayOfDouble & f1, const NOMAD::Double & h1, const NOMAD::ArrayOfDouble & f2, const NOMAD::Double & h2) const;


    /** Helper for updateWithPoints
     * Used for updating hMax
     * Get h just below hmax among all xInf
     */
    NOMAD::Double getWorstHInBarrier() const;
    
protected:

    /** Helper for updateWithPoints
        * Set the infeasible incumbent(s) from xInf
     */
    bool setInfeasibleIncumbents() ;


};




#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_PROGRESSIVEBARRIER__
