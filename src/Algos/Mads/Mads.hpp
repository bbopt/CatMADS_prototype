
#ifndef __NOMAD_4_5_MADS__
#define __NOMAD_4_5_MADS__

#include "../../Algos/Algorithm.hpp"
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/SearchMethodBase.hpp"

#include "../../nomad_nsbegin.hpp"

typedef std::function<bool(const Step& step, std::list<Direction> & dir, const size_t n)> UserPollMethodCbFunc;  ///< Type definitions for callback functions for user Poll method.
typedef std::function<bool(const Step& step, EvalPointSet & trialPoint)> UserSearchMethodCbFunc;  ///< Type definitions for callback functions for user Search method.
typedef std::function<bool(const Step& step)> UserMethodEndCbFunc;  ///< Type definitions for callback functions used after evaluations of trial points proposed by user Search and Poll methods.

class ExtendedPollMethod;

/// The (M)esh (A)daptive (D)irect (S)earch algorithm.
/**
\note AllParameters and EvaluatorControl are held by MainStep.
Cache is a singleton all by itself.
MegaIteration holds the algorithm-related structures: Mesh, Barrier.
 */
class Mads: public Algorithm
{
private:

    static UserSearchMethodCbFunc       _cbUserSearchMethod;
    static UserSearchMethodCbFunc       _cbUserSearchMethod_2;
    static UserMethodEndCbFunc          _cbUserSearchMethodEnd;
    static UserPollMethodCbFunc         _cbUserPollMethod;
    static UserPollMethodCbFunc         _cbUserFreePollMethod;
    static UserMethodEndCbFunc          _cbUserFreePollMethodEnd;

    // Flags for user method callbacks.
    // Flags are set to true when adding callback. This is done only if USER_CALLS_ENABLED==true.
    bool _hasUserSearchMethod, _hasUserPollMethod, _hasUserFreePollMethod;

    std::shared_ptr<ExtendedPollMethod> _extendedPollMethod = nullptr;
    
    
private:
    std::vector<std::pair<std::size_t,std::shared_ptr<SearchMethodBase>>> _extraSearchMethods;

public:
    /// Constructor
    /**
     \param parentStep          The parent of this step -- \b IN.
     \param stopReasons         The stop reasons for MADS -- \b IN.
     \param runParams           The run parameters that control MADS -- \b IN.
     \param pbParams            The problem parameters that control MADS -- \b IN.
     \param barrierInitializedFromCache  Flag to initialize barrier from cache or not -- \b IN.
     \param useOnlyLocalFixedVariables   Flag to use only local fixed variables or not (not the ones from the original problem) -- \b IN.
     */
    explicit Mads(const Step* parentStep,
                  std::shared_ptr<AlgoStopReasons<MadsStopType>> stopReasons,
                  const std::shared_ptr<RunParameters>& runParams,
                  const std::shared_ptr<PbParameters>& pbParams,
                  bool barrierInitializedFromCache = true,
                  bool useOnlyLocalFixedVariables = false )
      : Algorithm(parentStep, stopReasons, runParams, pbParams, useOnlyLocalFixedVariables),
    _hasUserSearchMethod(false),
    _hasUserPollMethod(false),
    _hasUserFreePollMethod(false)
    {
        init(barrierInitializedFromCache);
    }

    /// Helper for hot restart
    void hotRestartOnUserInterrupt() override;

    /// For suggest and observe PyNomad interface
    NOMAD::ArrayOfPoint suggest() override;
    void observe(const std::vector<NOMAD::EvalPoint>& evalPointList) override;
    
    
    /// Insert extra search methods. To be accesses
    void insertSearchMethod(size_t pos, const std::shared_ptr<SearchMethodBase>& searchMethod)
    {
        _extraSearchMethods.push_back(std::pair<size_t,std::shared_ptr<SearchMethodBase>>(pos,searchMethod));
    }
    
    std::vector<std::pair<std::size_t,std::shared_ptr<SearchMethodBase>>> & accessExtraSearchMethods()
    {
        return _extraSearchMethods;
    }

    /// \brief Set user method callback
    void addCallback(const CallbackType& callbackType,
                     const UserPollMethodCbFunc& userPollCbFunc);
    void addCallback(const CallbackType& callbackType,
                     const UserSearchMethodCbFunc& userSearchCbFunc);
    /// \brief Set user method post eval callback
    void addCallback(const CallbackType& callbackType,
                     const UserMethodEndCbFunc& userCbFunc) const;

    /// \brief Run user poll method callback to produce direction
    bool runCallback(const CallbackType& callbackType,
                     const Step& step,
                     std::list<Direction> & dir,
                     const size_t n) const;

    /// \brief Run user search method callback to produce trial points
    bool runCallback(const CallbackType& callbackType,
                     const Step& step,
                     EvalPointSet & trialPoints) const;

    /// \brief Run user method post eval callback to produce direction
    bool runCallback(const CallbackType& callbackType,
                     const Step& step) const;

    bool hasUserSearchMethod() const {return _hasUserSearchMethod;}
    bool hasUserPollMethod() const {return _hasUserPollMethod;}
    bool hasUserFreePollMethod() const {return _hasUserFreePollMethod;}
    
    ///\brief set for user extended poll method
    void setExtendedPollMethod(std::shared_ptr<ExtendedPollMethod> extendedPollMethod) { _extendedPollMethod = extendedPollMethod; }
    
    ///\brief access to the extended poil method
    std::shared_ptr<ExtendedPollMethod> getExtendedPollMethod() const { return _extendedPollMethod; }
    

private:
    ///  Initialization of class, to be used by Constructor.
    /**
    \param barrierInitializedFromCache  Flag to initialized barrier from cache or not -- \b IN.
    */
    void init(bool barrierInitializedFromCache);

    /// Algorithm execution for single-objective.
    /**
     Overrides the default algorithm's run
     \return \c true if a full success was found, \c false otherwise
     */
    virtual bool runImp() override;

    /// Helper for start()
    void readInformationForHotRestart() override;
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_MADS__
