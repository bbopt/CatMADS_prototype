/**
 * \file   CacheBase.hpp
 * \brief  Base class for cache
 * \author Viviane Rochon Montplaisir
 * \date   April 2017
 */

#ifndef __NOMAD_4_5_CACHEBASE__
#define __NOMAD_4_5_CACHEBASE__

#include <atomic>       // For atomic
#include <vector>

#include "../nomad_platform.hpp"
#include "../Eval/EvalPoint.hpp"
#include "../Param/CacheParameters.hpp"

#include "../nomad_nsbegin.hpp"


class CacheSet;

/// Abstract base class for cache
/**
 * \note Most methods of this are pure virtual and must be implemented in a derived class.
 *
 * The cache itself is implemented in derived classes.
 * It could be a set (CacheSet), an unordered_set (CacheSet with
 * precompiler option USE_UNORDEREDSET), map, multimap,
 * SQL database, etc.
 */
class CacheBase {

protected:

    /*---------*/
    /* Members */
    /*---------*/


    /// Number of times a point generated for evaluation was found in cache.
    /**
     * Used for computation of nbEval (parameter MAX_EVAL).
     * \note The cache itself is implemented in derived classes.
     *   It could be a set (CacheSet), an unordered_set (CacheSet with
     *   precompiler option USE_UNORDEREDSET) map, multimap, SQL database, etc.
    */
    DLL_EVAL_API static std::atomic<size_t> _nbCacheHits;

    /// Name of the file to write or read cache to.
    /**
     The format depends on the cache implementation. This is only the name
    of the file.
     */
    std::string _filename;


    /// Maximum number of points to be stored in the cache.
    /**
     \note Adding more points should remove older points.
     */
    size_t _maxSize;

    /// The cache parameters used by the cache
    std::shared_ptr<CacheParameters> _cacheParams;

    DLL_EVAL_API static std::unique_ptr<CacheBase> _single; ///< The singleton

    /// Dimension of the points in the cache.
    /**
     * Used for verification only.
     * To be reviewed when we address category variables. Maybe a
       signature will be needed.
     */
    size_t _n;

    /// Signals that the cache should stop waiting for points to be evaluated.
    /**
     * For methods that allow waiting for a point to be evaluated:
     * If the whole optimization is done, stop waiting for points
     * that will never be evaluated.
     */
    std::atomic<bool> _stopWaiting;

    /*---------*/
    /* Methods */
    /*---------*/

    /// Protected constructor.
    /**
     * Called only by derived object to instantiate the singleton
     \param cacheParams The cache parameters -- \b IN.
     */
    explicit CacheBase(const std::shared_ptr<CacheParameters>& cacheParams)
      : _cacheParams (cacheParams),
        _n(0),
        _stopWaiting(false)
    {
        init();
    }

public:
    /// Copy constructor not available
    CacheBase ( CacheBase const & ) = delete;

    /// Operator= not available
    CacheBase & operator= ( CacheBase const & ) = delete;


    /// Static function to access the singleton.
    /**
     \return The instance (singleton).
     */
    static const std::unique_ptr<CacheBase>& getInstance()
    {
        if (nullptr == _single)
        {
            std::string err = "Cannot get instance. A non-virtual object derived from CacheBase must be instantiated first. For example, call CacheSet::setInstance() ONCE before calling CacheBase::getInstance()" ;
            throw Exception(__FILE__, __LINE__, err);
        }

        return _single;
    }

    static void resetInstance()
    {
        if (nullptr != _single)
        {
            _single->clear();
            _single.release() ; // NOT SURE. No need to release the unique ptr. When running multiple time, with a singleton we have multiple unique_ptr created.
        }
    }
    
    /// Destructor
    virtual ~CacheBase(void) = default;



    /*---------*/
    /* Get/Set */
    /*---------*/
    static size_t getNbCacheHits() { return _nbCacheHits; }
    static void setNbCacheHits(size_t cacheHits) { _nbCacheHits = cacheHits; }

    static void resetNbCacheHits() { _nbCacheHits = 0; }

    void setFileName(const std::string &filename) { _filename = filename; }
    std::string getFileName() const { return _filename; }

    void setMaxSize(const size_t maxSize) { _maxSize = maxSize; }

    void setStopWaiting(const bool stopWaiting) { _stopWaiting = stopWaiting; }

    /*---------------*/
    /* Other methods */
    /*---------------*/

    /// Compute the mean f.
    /**
     \param mean  The mean of f -- \b IN.
     \return      The number of EvalPoints for which f is defined.
     */
    virtual size_t computeMeanF(Double &mean) const
    {
        std::cout << "Warning: computeMeanF is not implemented for this type of cache." << std::endl;
        mean.reset();
        return 0;
    }

    /// Process function func on all EvalPoints in the cache.
    /**
       Hypothesis: The Point part is not affected, and this does
       not affect the sorting order of the cache. This means that
       each EvalPoint may be processed in place. It is not needed
       to remove it from the cache, process it, and then put it back.
     */
    typedef void (*EvalFunc_t)(EvalPoint&);
    virtual void processOnAllPoints(EvalFunc_t NOMAD_UNUSED(func), const int NOMAD_UNUSED(mainThreadNum) = -1)
    {
        std::cout << "Warning: processOnAllPoints is not implemented for this type of cache." << std::endl;
    }

    virtual void deleteModelEvalOnly(const int mainThreadNum) = 0;



    /// Add a new EvalPoint to the cache.
    /**
     * If insertion worked, the point was not in the cache before. Return true.\n
     * If insertion did not work, the point was in the cache before.
       \c _nbCacheHits is incremented. Return false.
       \note This implementation calls smartInsert().
     \param  evalPoint   The eval point to insert in cache -- \b IN.
     \return             \c true if insertion works and \c false if not.
     */
    virtual bool insert(const EvalPoint &evalPoint) = 0;

    /// Find eval point in cache.
    /**
     Get first eval point at point x from the cache.

     \param x           The point to find                       -- \b IN.
     \param evalPoint   The returned eval point that matches x  -- \b IN/OUT.
     \param evalType    If not UNDEFINED, wait for the point to have an evaluation for this evaltype. -- \b IN.
     \param waitIfNotYetAvailable    Flag to control if we wait for the point to have an evaluation for this evaltype. -- \b IN.
     \return            The number of eval points found.
     */
    virtual size_t find(const Point & x, EvalPoint &evalPoint,
                        EvalType evalType = EvalType::UNDEFINED,
                        bool waitIfNotYetAvailable = true ) const = 0;
    
    /// Get eval point at point x from the cache for rerun (there can be only one in CacheSet).
    /**
     \param x           The point to find                   -- \b IN.
     \param evalPoint   The returned eval point that matches x  -- \b IN/OUT.
     \return true if the evalPoint  found in cache for rerun, false otherwise.
     */
    virtual bool findInCacheForRerun(const Point & x,
                                    NOMAD::EvalPoint &evalPoint ) const = 0;
    

    /// Insert evalPoint in cache.
    /**
     * evalPoint's tag (mutable) is updated.
     * If insertion worked, the point was not in the cache before. Return true.
     * If insertion did not work, the point was in the cache before.
      _nbCacheHits is incremented.
     * Depending on its EvalStatus, on maxNumberEval, and on the evalType,
       return \c true if it should be evaluated again,
       \c false otherwise.
     \param evalPoint       The eval point to insert in the cache -- \b IN.
     \param maxNumberEval   The max number of evals           -- \b IN.
     \param evalType        Which eval of the EvalPoint to look at -- \b IN.
     \return                A boolean indicating if we should eval this point.
     */
    virtual bool smartInsert(const EvalPoint &evalPoint,
                             const short maxNumberEval = 1,
                             EvalType evalType = EvalType::BB) = 0;

    /// Find all eval points at point x in the cache.
    /**
     Get all eval points at point x from the cache (pure virtual function).

     \param x               The point to find in cache             -- \b IN.
     \param evalPointList   The list of eval points found in cache -- \b OUT.
     \return                The number of points found.
     */
    virtual size_t find(const Point& x,
                        std::vector<EvalPoint> &evalPointList) const = 0;


    /// Get all eval points for which comp(refeval) returns true.
    /**
     The comparison function tests if an eval point's eval is inferior to refeval.
     \param refeval         The point of reference                                      -- \b IN.
     \param comp            The comparison function                                     -- \b IN.
     \param computeType   Which type of f, h computation (eval type, compute type and h norm type)  -- \b IN.
     \return                The number of points found.
     */
    virtual size_t find(const Eval &refeval,
                        std::function<bool(const Eval&, const Eval&, const FHComputeTypeS&)> comp,
                        std::vector<EvalPoint> &evalPointList,
                        const FHComputeType& computeType) const = 0;


    /// Get best eval points, using comp(). Only the points with eval status EVAL_OK are considered.
    /**
     \param comp            The comparison function                                    -- \b IN.
     \param evalPointList   The best eval points that verify comp()==true  in a list   -- \b OUT.
     \param findFeas        The flag to find feasible points                           -- \b IN.
     \param hMax            The hmax to detect feasibility                             -- \b IN.
     \param fixedVariable   Searching for a subproblem defined by this point           -- \b IN.
     \param computeType   Which type of f, h computation (eval type, compute type and h norm type)  -- \b IN.
     \return                The number of eval points found.
     */
    virtual size_t findBest(std::function<bool(const Eval&,
                                               const Eval&,
                                               const FHComputeTypeS&)> comp,
                            std::vector<EvalPoint> &evalPointList,
                            const bool findFeas,
                            const Double& hMax,
                            const Point& fixedVariable,
                            const FHComputeType& computeType) const = 0;


    /// Test if cache contains feasible points.
    /**
      \return \c true if the cache contains at least one feasible point, \c false otherwise.
     */
    virtual bool hasFeas(const FHComputeType& completeComputeType) const = 0;
    
    /// Test if cache contains an infeasible points.
    /**
      \return \c true if the cache contains at least one infeasible point, \c false otherwise.
     */
    virtual bool hasInfeas(const FHComputeType& completeComputeType) const = 0;


    /// Get all eval points within a distance of point X.
    /**
     \param X               The point of reference                              -- \b IN.
     \param crit            The criteria function                               -- \b IN.
     \param evalPointList   The eval points within the prescribed distance of X -- \b OUT.
     \param maxEvalPoints   The maximum number of points to select              -- \b IN.
     \return                The number of eval points found.
     */
    virtual size_t find(const Point & X,
                        std::function<bool(const Point&, const EvalPoint &)> crit,
                        std::vector<EvalPoint> &evalPointList,
                        int maxEvalPoints = 0) const = 0;



    /// Find using criteria.
    /**
     All the points for which crit() return \c true are put in evalPointList.

     \param crit            The criteria function                               -- \b IN.
     \param evalPointList   The eval points within the prescribed distance of X -- \b OUT.
     \return                The number of eval points found.
     */
    virtual size_t find(std::function<bool(const EvalPoint&)> crit,
                        std::vector<EvalPoint> &evalPointList) const = 0;

    /// Browse cache using criteria. The function can have access to remote info using the lambda
    /// function capture by reference.
    /**
    \param crit            The criteria function                               -- \b IN.
    */
    virtual void browse(std::function<void(const EvalPoint&)> crit) const =0;

    /// Get all eval points using two custom criteria.
    /**
     All the points for which the two crit() functions return \c true are put in evalPointList.

     \param crit1           The first criteria function                             -- \b IN.
     \param crit2           The second criteria function                            -- \b IN.
     \param evalPointList   The eval points within the prescribed distance of X     -- \b OUT.
     \return                The number of eval points found.
     */
    virtual size_t find(std::function<bool(const EvalPoint&)> crit1,
                        std::function<bool(const EvalPoint&)> crit2,
                        std::vector<EvalPoint> &evalPointList) const = 0;


    /// Get all non dominated (or equal) best feasible eval points using dominance criterion
    /// Used for multiobjective optimization
    /// NB: To use with precaution, computationally costly (n log n for two objectives).
    /**
     \param evalPointList   The best non dominated feasible eval points in a list  -- \b OUT.
     \param fixedVariable   Searching for a subproblem defined by this point -- \b IN.
     \param computeType   Which type of f, h computation (eval type, compute type and h norm type)  -- \b IN.
     \return                The number of eval points found.
     */
    virtual size_t findBestFeas(std::vector<EvalPoint> &evalPointList,
                                const Point& fixedVariable = Point(),
                                const FHComputeType& computeType  = defaultFHComputeType) const = 0;


    /// Find best infeasible points with h<=hmax:
    ///  -> index 0 and above if duplicates, least infeasible point with smallest f
    ///  -> last index and below if duplicates, best f with smallest h
    /// All best f points have the same blackbox outputs. Idem for the least infeasible points.
    /// Works also for multiobjective optimization
    /**
     \param evalPointList   The best infeasible eval points   -- \b OUT.
     \param fixedVariable   Searching for a subproblem defined by this point -- \b IN.
     \param hMax            Select a point if h <= hMax                                                 -- \b IN.
     \param computeType   Which type of f, h computation (eval type, compute type and h norm type)  -- \b IN.
     \return                The number of eval points found.
     */
    virtual size_t findBestInf(std::vector<EvalPoint> &evalPointList,
                               const Double& hMax = INF ,
                               const Point& fixedVariable = Point(),
                               const FHComputeType& computeType = defaultFHComputeType) const = 0;


    /// Get all non dominated (or equal) infeasible eval points using dominance criterion
    /// Used for multiobjective optimization
    /// NB: To use with precaution, computationally costly (O(n^2 m) where n is the number
    /// of points in the cache and m the number of objectives)
    /**
     \param evalPointList   The best non dominated feasible eval points in a list  -- \b OUT.
     \param fixedVariable   Searching for a subproblem defined by this point -- \b IN.
     \param hMax            Select a point if h <= hMax                                                 -- \b IN.
     \param computeType   Which type of f, h computation (eval type, compute type and h norm type)  -- \b IN.
     \return                The number of eval points found.
     */
    virtual size_t findFilterInf(std::vector<NOMAD::EvalPoint> &evalPointList,
                                 const Double& hMax,
                                 const Point& fixedVariable,
                                 const FHComputeType& computeType) const = 0;


    // More find() methods can be added here.
    // Find on multiple points -returning multiple eval points
    // Etc.


    /// Get all points of the cache and put them into a list.
    /**
     \param evalPointList   The list of all eval points -- \c OUT.
     \return                The dimension of the list.
     */
	DLL_EVAL_API size_t getAllPoints(std::vector<EvalPoint> &evalPointList) const;

    /// Update EvalPoint in cache.
    /**
     * Look for Point and update the Eval part.\n
     * Eval is assumed non-NULL. \n
     * If the point is not found, throw an exception.
     \param evalPoint       The eval point to update                                        -- \b IN.
     \param evalType        Which eval of the EvalPoint to look at -- \b IN.
     \param mesh                 The mesh to put in the EvalPoint (can be nullptr) -- \b IN.
     \return                A boolean indicating if update succeeded (\c true), \c false if there was an error.
     */
    virtual bool update(const EvalPoint& evalPoint, EvalType  evalType, const MeshBasePtr mesh = nullptr) = 0;

    /// Return number of eval points in the cache.
    virtual size_t size() const = 0;

    /// Empty the cache (pure virtual).
    virtual bool clear() = 0;

    /// Clear all quad and sgtelib evaluations from the cache.
    virtual void clearModelEval(const int mainThreadNum) = 0;

    /**
     * \brief Purge the cache from elements with higher f.
     *
     * The goal is to get under CACHE_SIZE_MAX EvalPoints in the cache.
     */
    virtual void purge()
    {
        std::cout << "Warning: purge is not implemented for this type of cache." << std::endl;
    }

    /**
     * \brief Write cache to file.
     *
     * Simple dump.
     */
    virtual bool write() const = 0;

    /**
     * \brief Display all points in cache.
     *
     * \note Used for debugging.
     */
    virtual std::string displayAll() const { return ""; }

    /// Read a cache file and load it.
    virtual bool read() = 0;
    
    
    /// Move eval points from cache set to cache set for rerun
    virtual void moveEvalPointToCacheForRerun() = 0;


private:

    /// Initialize the cache.
    /**
     * The initialization uses the parameters from a private CacheParameters object.
     */
    void init();
};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_CACHEBASE__
