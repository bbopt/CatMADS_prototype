#ifndef __NOMAD_4_5_BARRIERBASE__
#define __NOMAD_4_5_BARRIERBASE__

#include "../Eval/EvalPoint.hpp"

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

/// Generic class for barrier following algorithm 12.2 of DFBO.
class DLL_EVAL_API BarrierBase
{
protected:

    std::vector<EvalPointPtr> _xFeas;  ///< Current feasible incumbent solutions
    std::vector<EvalPointPtr> _xInf;   ///< Current infeasible barrier points (contains infeasible incumbents) with h<=hMax
    
    std::vector<EvalPointPtr> _xIncFeas;   ///< Current feasible incumbent solutions. Can be a subset of _xFeas but for now, xIncFeas and xFeas are the same
    std::vector<EvalPointPtr> _xIncInf;   ///< Current infeasible incumbent solutions (subset of _xInf if it is defined). For now, we consider a vector (maybe DMultiMads needs it)
    
    EvalPointPtr _refBestFeas;      ///< Previous first feasible incumbent
    EvalPointPtr _refBestInf;       ///< Previous first infeasible incumbent
                                    ///< NB: can be above the barrier threshold
    
    Double _hMax;                   ///< Maximum acceptable value for
    
    /// Attributes to define the computation of f and h
    FHComputeType _computeType;
    
    
    /// Dimension of the points in the barrier.
    /**
     * Used for verification only.
     * To be reviewed when we address category variables.
       /see _n in CacheBase.
     */
    size_t _n;

public:
    /// Constructor
    /**
     * hMax will be updated during optimization.
     \param hMax            The max of h to keep a point in the barrier -- \b IN.
     */
    BarrierBase(const EvalType &evalType, const FHComputeTypeS& computeType, const Double& hMax = INF)
      : _hMax(hMax),
        _computeType{evalType, computeType},
        _n(0)
    {}

    /// Copy Constructor
    /**
       Copy barrier some parameters. Barrier points are not copied.
     */
    BarrierBase(const BarrierBase & b)
    {
        _hMax = b._hMax;
        _computeType = b._computeType;
    }
    
    // Use clone to create a barrier of the same type (for example, ProgressiveBarrier, DiscoMadsBarrier or DMultiMadsBarrier)
    virtual std::shared_ptr<BarrierBase> clone() const = 0;
    
    /*-----------------*/
    /* Feasible points */
    /*-----------------*/
    /// Get all feasible points in the barrier
    /**
     \return All the eval points that are feasible.
     */
    const std::vector<EvalPointPtr>& getAllXFeas() const { return _xFeas; }
    
    ///  Get the current incumbent feasible point in the barrier.
    /**
     * If there is no feasible point, return a \c nullptr
     \return A single feasible eval point.
     */
    virtual EvalPointPtr getCurrentIncumbentFeas() const =0;
    
    
    ///  Get all incumbent feasible points in the barrier
    /**
     \return All the eval points that are feasible incumbents (with same f and h).
     */
    const std::vector<EvalPointPtr>& getAllXIncFeas() const { return _xIncFeas; }
    
    ///  Get the point that was previously the first feasible point in the barrier.
    /**
     * If there is no feasible point, return a \c nullptr
     \return A single feasible eval point.
     */
    EvalPointPtr getRefBestFeas() const { return _refBestFeas; }
    void setRefBestFeas(const EvalPointPtr refBestFeas) { _refBestFeas = refBestFeas; }
    
    /// Update ref best feasible and ref best infeasible values.
    virtual void updateRefBests() = 0;

    /// Number of feasible points in the barrier.
    size_t nbXFeas() const {return _xFeas.size();}

    /// Remove feasible points from the barrier.
    virtual void clearXFeas();

    /*-------------------*/
    /* Infeasible points */
    /*-------------------*/
    ///  Get all infeasible points in the barrier
    /**
     \return All the eval points that are infeasible.
     */
    const std::vector<EvalPointPtr>& getAllXInf() const { return _xInf; }
    
    ///  Get all incumbent infeasible points in the barrier
    /**
     \return All the eval points that are infeasible incumbents (with same f and h).
     */
    const std::vector<EvalPointPtr>& getAllXIncInf() const { return _xIncInf; }

    
    ///  Get the current infeasible incumbent.
    /**
     * If there is no infeasible point, return a \c nullptr
     \return A single infeasible eval point.
     */
    virtual EvalPointPtr getCurrentIncumbentInf() const = 0;
    
    
    ///  Get the point that was previously the first infeasible point in the barrier.
    /**
     * If there is no feasible point, return a \c nullptr
     \return A single feasible eval point.
     */
    EvalPointPtr getRefBestInf() const { return _refBestInf; }
    void setRefBestInf(const EvalPointPtr refBestInf) { _refBestInf = refBestInf; }
    
    /// Number of infeasible points in the barrier.
    size_t nbXInf() const { return _xInf.size() ;}

    /// Remove infeasible points from the barrier.
    virtual void clearXInf() ;

    /*---------------*/
    /* Other methods */
    /*---------------*/
    /// Get all feasible and infeasible points ptr
    std::vector<EvalPoint> getAllPoints() const ;
    
    /// Make a copy of all feasible and infeasible points
    std::vector<EvalPointPtr> getAllPointsPtr() const ;

    /// Get first of all feasible and infeasible points.
    /** If there are feasible points, returns first feasible point.
      * else, returns first infeasible incumbent. */
    const EvalPointPtr getFirstPoint() const;
    
    /// Get the current hMax of the barrier.
    Double getHMax() const { return _hMax; }

    /// Set the hMax of the barrier
    /**
     \param hMax    The hMax -- \b IN.
     */
    virtual void setHMax(const Double &hMax) = 0;

    ///  xFeas and xInf according to given points.
    /* \param evalPointList vector of EvalPoints  -- \b IN.
     * \param keepAllPoints keep all good points, or keep just one point as in NOMAD 3 -- \b IN.
     * \return true if the Barrier was updated, false otherwise
     * \note Input EvalPoints are already in subproblem dimension
     */
    virtual SuccessType getSuccessTypeOfPoints(const EvalPointPtr xFeas,
                                               const EvalPointPtr xInf) = 0;

    /// Update xFeas and xInf according to given points.
    /* \param evalPointList vector of EvalPoints  -- \b IN.
     * \param keepAllPoints keep all good points, or keep just one point as in NOMAD 3 -- \b IN.
     * \return true if the Barrier was updated, false otherwise
     * \note Input EvalPoints are already in subproblem dimension
     */
    virtual bool updateWithPoints(
                          const std::vector<EvalPoint>& evalPointList,
                          const bool keepAllPoints = false,
                          const bool updateInfeasibleIncumbentAndHmax = false) = 0;
    
    /// Return the barrier as a string.
    /* May be used for information, or for saving a barrier. In the former case,
     * it may be useful to set parameter max to a small value (e.g., 4). In the
     * latter case, INF_SIZE_T should be used so that all points are saved.
     * \param max Maximum number of feasible and infeasible points to display
     * \return A string describing the barrier
     */
    virtual std::vector<std::string> display(const size_t max = INF_SIZE_T) const =0;
    
    
    bool findPoint(const Point & point, EvalPoint & foundEvalPoint) const;
    
    void checkForFHComputeType(const FHComputeType& computeType) const;
    
    /// Access to f and h computation
    HNormType getHNormType() const {return _computeType.fhComputeTypeS.hNormType; }
    ComputeType getComputeType() const { return _computeType.fhComputeTypeS.computeType;}
    EvalType getEvalType() const { return _computeType.evalType; }
    const FHComputeType& getFHComputeType() const { return _computeType;}
    
protected:
    
    /**
     * \brief Helper function for init/constructor.
     */
    void setN();
    
    /**
     * \brief Helper function for init/setHMax.
     *
     * Will throw exceptions or output error messages if something is wrong. Will remain silent otherwise.
     */
    void checkHMax();

    /**
     * \brief Helper function for init/constructor.
     *
     * Throw an exception if the Cache has not been instantiated yet. Will remain silent otherwise.
     */
    void checkCache();
    
    
    std::vector<EvalPointPtr>::iterator findEvalPoint(std::vector<EvalPointPtr>::iterator first, std::vector<EvalPointPtr>::iterator last, const EvalPoint & p  );
    
private:

    /**
     * \brief Helper function for constructor.
     *
     * Will throw exceptions or output error messages if something is wrong. Will remain silent otherwise.
     \param fixedVariable   The fixed variables have a fixed value     -- \b IN.
     \param barrierInitializedFromCache  Flag to initialize barrier from cache or not. -- \b IN.
     */
    virtual void init(const Point& fixedVariable,
                      bool barrierInitializedFromCache) = 0;
    
    /**
     * \brief Helper function for insertion.
     *
     * Will throw exceptions or output error messages if something is wrong. Will remain silent otherwise.
     */
    void checkXFeas(const EvalPoint &xFeas) ;
    
    /**
     * \brief Helper function for insertion.
     *
     * Will throw exceptions or output error messages if something is wrong. Will remain silent otherwise.
     */
    virtual void checkXFeasIsFeas(const EvalPoint &xFeas);
    
    /**
     * \brief Helper function for insertion.
     *
     * Will throw exceptions or output error messages if something is wrong. Will remain silent otherwise.
     */
    void checkXInf(const EvalPoint &xInf) const;
    
    
};

/// Display useful values so that a new Barrier could be constructed using these values.
DLL_EVAL_API std::ostream& operator<<(std::ostream& os, const BarrierBase& barrier);

/// Get barrier values from stream
DLL_EVAL_API std::istream& operator>>(std::istream& is, BarrierBase& barrier);

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_BARRIERBASE__
