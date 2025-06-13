#ifndef __NOMAD_4_5_EXTENDEDPOLL__
#define __NOMAD_4_5_EXTENDEDPOLL__

#include "../../Algos/Mads/ExtendedPollMethod.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to manage the method used by MADS algorithm during its extended poll step.
class ExtendedPoll final : public Step , public IterationUtils
{
private:
    std::shared_ptr<ExtendedPollMethod> _extendedPollMethod = nullptr; 
    bool _isEnabled = false;
    
#ifdef TIME_STATS
    DLL_ALGO_API static std::vector<double> _extendedPollTime;        ///< Total time spent running each extended poll
    DLL_ALGO_API static std::vector<double> _extendedPollEvalTime;    ///< Total time spent evaluating extended poll points
#endif // TIME_STATS

public:
    /// Constructor
    /**
     /param parentStep      The parent of this extended poll step -- \b IN.
     */
    explicit ExtendedPoll(const Step* parentStep )
      : Step( parentStep ),
        IterationUtils( parentStep )
    {
        init();
    }

    virtual ~ExtendedPoll() {}

#ifdef TIME_STATS
    /// Time stats
    static std::vector<double> getExtendedPollTime()       { return _extendedPollTime; }
    static std::vector<double> getExtendedPollEvalTime()   { return _extendedPollEvalTime; }
#endif // TIME_STATS
    
    /**
     Identify if the extended poll method exists and is enabled.
     */
    bool isEnabled() const {return _isEnabled;}
    
private:

    void init();

    /// Implementation of the start task.
    /**
     Just perform a sanity check on MEGA_SEARCH_POLL that must be false.
     */
    virtual void startImp() override;

    /// The implementation of run tasks.
    /**
      Perform start+run+end for the extended poll method.
     */
    virtual bool runImp() override;

    /// Implementation of the end tasks
    /**
      If a sub optimization is used during extended poll we probably set a stop reason to terminate. The parent optimization must go on. The stop reason is set to started if sub optimization reached its evaluation budget.
     */
    virtual void endImp() override;
    
    /**
     - Extended poll requires information from poll and search. Cannot generated trial points
     when parameter MEGA_SEARCH_POLL is true.
     */
    void generateTrialPointsImp() override {};
    
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_EXTENDEDPOLL__

