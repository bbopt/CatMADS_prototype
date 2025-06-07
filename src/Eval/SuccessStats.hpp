#ifndef __NOMAD_4_5_STEPSUCCESSSTATS__
#define __NOMAD_4_5_STEPSUCCESSSTATS__

#include <map>

#include "../Type/StepType.hpp"
#include "../Util/Exception.hpp"
#include "../Util/utils.hpp"

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

/// Class to manage success type stats of step
/**
  Stats for success type of steps.
  Some stats are propagated from step to parent step (nbSuccessAndFail) but the number of consecutive successes and fails of a step is not propagated.
 */
class DLL_EVAL_API SuccessStats
{
protected:
    
    // This success type is propagated from step to parent step.
    std::map<std::pair<StepType,SuccessType>, size_t> _nbSuccessAndFail;
    
    // Success: PARTIAL_SUCCESS and FULL_SUCCESS
    // Fail: NO_TRIALS, UNSUCCESSFUL
    // UNDEFINED success type is not recorded
    // This is not propagated from step to parent step. No need to have a map with StepType
    size_t _nbConsecutiveSuccess;
    size_t _nbConsecutiveFail;
    
public:
    /// Constructor
    explicit SuccessStats():
    _nbConsecutiveSuccess(0),
    _nbConsecutiveFail(0)
    {}
       
    /// Update the stats for a given success type and step.
    void updateStats(SuccessType successType, StepType stepType , size_t val=1);
    
    /// Update the stats with a given success stats
    void updateStats(const SuccessStats & evalStats);
    
    /// Access to consecutive successes and fails
    size_t getStatsNbConsecutiveSuccess() const { return _nbConsecutiveSuccess ; }
    size_t getStatsNbConsecutiveFail() const { return _nbConsecutiveFail ; }
    
    /// Access to stat for given step type and success type
    size_t getStat(StepType stepType, SuccessType successType) const;
    
    
    // size_t getNbSuccessAndFail(SuccessType successType, StepType stepType) const;
     
    // Reset stats that are passed to parents
    void resetCurrentStats() { _nbSuccessAndFail.clear(); } ///< Reset the current stats for all registered step types. Resets are performed at start of step.
    
    bool hasStatsForPropagation() const { return !_nbSuccessAndFail.empty();} 
    
    std::string display() const;
    
private:
    
    const std::map<std::pair<StepType,SuccessType>, size_t> & getStatsMapSuccessAndFail() const { return _nbSuccessAndFail ;}
    
    void setNbConsecutiveSuccessAndFail(SuccessType successType, size_t val);
    
    /// Update the stats for a given success type and step.
    void updateSuccessAndFailStats(SuccessType successType, StepType stepType , size_t val);
    
    
};
///   Display method stats.
std::ostream& operator<<(std::ostream& os, const SuccessStats& stats);


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_BASEEVALSTATS__
