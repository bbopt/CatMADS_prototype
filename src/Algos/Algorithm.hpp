
#ifndef __NOMAD_4_5_ALGORITHM__
#define __NOMAD_4_5_ALGORITHM__

#include "../Algos/EvcInterface.hpp"
#include "../Algos/Initialization.hpp"
#include "../Algos/MegaIteration.hpp"
#include "../Algos/Step.hpp"
#include "../Algos/Termination.hpp"
#include "../Algos/TrialPointStats.hpp"

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../nomad_nsbegin.hpp"


/// Generic class for any direct search optimizer algorithm
/**
  \note: AllParameters and EvaluatorControl are held by MainStep.
 \note: Cache is a singleton all by itself.
 \note MegaIteration holds the algorithm-related structures.
 */
class Algorithm: public Step
{
private:
    bool _isSubAlgo;

protected:

    std::unique_ptr<Initialization>  _initialization;   ///< To initialize the algorithm (X0)
    std::unique_ptr<Termination>     _termination;      ///< To verify termination conditions
    std::shared_ptr<MegaIteration>   _refMegaIteration; ///< MegaIteration used to pass information between two algorithm runs

    bool                             _endDisplay;

    bool                             _algoSuccessful;

#ifdef TIME_STATS
    size_t _totalRealAlgoTime;
    double _startTime;
    double _totalCPUAlgoTime;
#endif // TIME_STATS

    TrialPointStats                        _trialPointStats;   ///< The trial point counters stats for algo execution
    
    bool _useOnlyLocalFixedVariables ; ///< When this flag is true, we force an algo to use only local fixed variable. The original problem fixed variables are not considered. This is useful when we change the design space like when doing quad model search. The evaluation of the quad model are only in the sub space and maybe there are some local fixed variables.
    
    bool _evalOpportunistic;  ///< This flag is used to force non opportunistic eval for some algo. The evaluator control function setOpportunisticEval is called with this flag. The parameter EVAL_OPPORTUNISTIC can be temporarily superseded (example, LH_EVAL + MADS)
    
    
public:
    /// Constructor
    /**
     \param parentStep           The parent of this Step -- \b IN.
     \param stopReasons         The stop reasons of this algo -- \b IN.
     \param runParams             The run parameters that control the algorithm -- \b IN.
     \param pbParams               The problem parameters that control the algorithm -- \b IN.
     \param useOnlyLocalFixedVariables Flag is to force an algo to consider only local fixed variables, not the ones from the original pb  -- \b IN.
     */
    explicit Algorithm(const Step* parentStep,
                       std::shared_ptr<AllStopReasons> stopReasons,
                       const std::shared_ptr<RunParameters>& runParams,
                       const std::shared_ptr<PbParameters>& pbParams ,
                       bool useOnlyLocalFixedVariables = false)
      : Step(parentStep, stopReasons, runParams, pbParams),
        _isSubAlgo(false),
        _initialization(nullptr),
        _termination(nullptr),
        _refMegaIteration(nullptr),
        _endDisplay(true),
#ifdef TIME_STATS
        _totalRealAlgoTime(0),
        _startTime(0.0),
        _totalCPUAlgoTime(0.0),
#endif // TIME_STATS
        _trialPointStats(parentStep),
        _useOnlyLocalFixedVariables(useOnlyLocalFixedVariables),
        _evalOpportunistic(true)
    {
        init();
    }

    /// Destructor
    virtual ~Algorithm();

    /*---------*/
    /* Get/Set */
    /*---------*/
    const std::shared_ptr<MegaIteration>& getRefMegaIteration() const { return _refMegaIteration; }
    void setRefMegaIteration(const std::shared_ptr<MegaIteration> megaIteration) { _refMegaIteration = megaIteration; }

    void setEndDisplay( bool endDisplay ) { _endDisplay = endDisplay; }
    
    void updateStats(TrialPointStats & trialPointStats); /// Update the trial point counter stats
    
    // Utility function to get BB_OUTPUT_TYPE parameter, which is buried in Evaluator.
    static BBOutputTypeList getBbOutputType()
    {
        if (nullptr == EvcInterface::getEvaluatorControl())
        {
            // Do not trigger an exception. Simply return an empty vector.
            return NOMAD::BBOutputTypeList();
        }
        return EvcInterface::getEvaluatorControl()->getCurrentBBOutputTypeList();
    }
    static size_t getNbObj();
    
    void setEvalOpportunistic(bool evalOpportunistic) { _evalOpportunistic = evalOpportunistic ;}
    bool getEvalOpportunistic() const { return _evalOpportunistic; }
    
protected:


    /// Default implementation of the start tasks of an algorithm
    /**
     If doing a hot restart get the algorithm ready to continue. \n
     If starting a new algorithm, reset the stop reason, the lap evaluation counter, and perform initialization.
     */
    void startImp() override;

    /// Default implementation of the end tasks of an algorithm
    /**
     Display some information, reset the lap counters, set success type for a search method and save information for a potential hot restart.
     */
    virtual void endImp() override;

    /// Each algorithm must implement its run tasks.
    /**
     Run algorithm execution for single-objective.
     \return \c true
     */
    virtual bool runImp() override = 0;

    /// Helper for start() when doing a hot restart.
    virtual void readInformationForHotRestart() = 0;

    /// Helper for end()
    void saveInformationForHotRestart() const;
    /// Helper for end()
    void displayBestSolutions() const;
    /// Helper for end()
    void displayEvalCounts() const;
    
    /// Helper for hot restart
    void hotRestartOnUserInterrupt() override;

public:
    /**
     Sub-algo: an algorithm can be part of an algorithm.
     */
    bool isSubAlgo() const { return _isSubAlgo; };
    bool isRootAlgo() const { return !_isSubAlgo; }

    /*---------*/
    /* Others  */
    /*---------*/
    /// Verify if this Algorithm is ready to be terminated
    bool terminate(size_t iteration);

    virtual void read(std::istream& is);
    virtual void display(std::ostream& os) const;

    /// Access to the best solution (can be undefined)
    virtual EvalPoint getBestSolution (bool bestFeas = false) const;
    
private:

    ///  Helper for Constructor.
    void init();
    
    
    private:
    
    /// Implementation to increment the nb of calls counter
    virtual void incrementCounters() override { _trialPointStats.incrementNbCalls() ; }

};

/// Operator to write parameters used for hot restart.
std::ostream& operator<<(std::ostream& os, const Algorithm& algo);

/// Operator to read parameters used for hot restart.
std::istream& operator>>(std::istream& is, Algorithm& algo);


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_ALGORITHM__
