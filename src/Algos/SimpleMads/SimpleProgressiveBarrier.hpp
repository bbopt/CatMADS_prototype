#ifndef __NOMAD_4_5_SIMPLEPROGRESSIVEBARRIER__
#define __NOMAD_4_5_SIMPLEPROGRESSIVEBARRIER__

#include "../../Eval/BarrierBase.hpp"
#include "../../Eval/EvalPoint.hpp"

#include "../SimpleMads/SimpleEvalPoint.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for single objective progressive barrier (for Mads).
/// Specific barrier points and incumbent for faster F, H access (SimpleEvalPoint).
class DLL_ALGO_API SimpleProgressiveBarrier
{
private:
    bool _incumbentsAndHMaxUpToDate;

    SimpleEvalPoint _xFeas;               ///< Current feasible incumbent solutions
    std::vector<SimpleEvalPoint> _xInf;   ///< Current infeasible barrier points (contains infeasible incumbents) with h<=hMax

    SimpleEvalPoint _xIncFeas;   ///< Current feasible incumbent solution. xIncFeas and xFeas are the same
    SimpleEvalPoint _xIncInf;   ///< Current infeasible incumbent solution

    SimpleEvalPoint _refBestFeas;   ///< Current reference feasible incumbent solutions.
    SimpleEvalPoint _refBestInf;   ///< Current reference infeasible incumbent solutions

    std::vector<SimpleEvalPoint> _xTmpInf;   ///< Added infeasible incumbents h<=hMax

    NOMAD::Double _hMax;

public:
    /// Constructor
    /**
     * hMax will be updated during optimization.
     \param hMax            The max of h to keep a point in the barrier -- \b IN.
     \param fixedVariable   The fixed variables have a fixed value -- \b IN.
     \param evalPointList   Additional points to consider in building the barrier -- \b IN.
     */
    SimpleProgressiveBarrier(const Double& hMax,
                             const Point& fixedVariable ,
                             const std::vector<SimpleEvalPoint>& evalPointList) :
        _incumbentsAndHMaxUpToDate(false),
        _hMax(NOMAD::INF)
    {
        init(fixedVariable,evalPointList);
    }



    // Copy constructor
    SimpleProgressiveBarrier(const SimpleProgressiveBarrier & b)
    {
        // Do not copy the barrier points. Do not initialize from cache.
    }


    /*-----------------*/
    /* Feasible points */
    /*-----------------*/

    /// Update ref best feasible and ref best infeasible values.
    void updateRefBests();


    /*---------------*/
    /* Other methods */
    /*---------------*/

    /// Update xFeas and xInf according to given points.
    /* \param evalPointList vector of EvalPoints  -- \b IN.
     * \note Input EvalPoints are already in subproblem dimension
     */
    bool updateWithPoints(const std::vector<SimpleEvalPoint>& evalPointList ) ;

    // Version that keep at most two points in the barrier
    bool updateWithPointsKeep2(const std::vector<SimpleEvalPoint>& evalPointList ) ;

    const SimpleEvalPoint & getCurrentIncumbentInf() const {return _xIncInf;}
    const SimpleEvalPoint & getCurrentIncumbentFeas() const {return _xIncFeas;}

    const SimpleEvalPoint & getRefBestInf() const {return _refBestInf;}
    const SimpleEvalPoint & getRefBestFeas() const {return _refBestFeas;}

    NOMAD::SuccessType computeSuccessType(const NOMAD::SimpleEvalPoint & p1, const NOMAD::SimpleEvalPoint & p2) const;


private:

    /**
     * \brief Helper function for constructor.
     *
     * Will throw exceptions or output error messages if something is wrong. Will remain silent otherwise.
     \param fixedVariable   The fixed variables have a fixed value     -- \b IN.
     \param evalPointList   Additional points to consider to construct barrier. -- \b IN.
     */
    void init(const Point& fixedVariable,
              const std::vector<SimpleEvalPoint>& evalPointList);


    /** Helper for updateWithPoints
     *
     */
    bool dominates(const NOMAD::SimpleEvalPoint & p1, const NOMAD::SimpleEvalPoint & p2) const;


    /** Helper for updateWithPoints
     * Used for updating hMax
     * Get h just below hmax among all xInf
     */
    NOMAD::Double getWorstHInBarrier() const;


    /** Helper for updateWithPoints
        * Set the infeasible incumbent(s) from xInf
     */
    bool setInfeasibleIncumbents() ;

    /** Helper for updateWithPoints
        * Remove dominated points from xInf
     */
    bool removeInfeasibleDominatedPoints();


    // Functions to reduce the number of points in barrier
    bool updateBarrierPointsKeep2();
    void reduceBarrierInfeasible();


};




#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SIMPLEPROGRESSIVEBARRIER__
