#ifndef __NOMAD_4_5_TRIALPOINTSTATS__
#define __NOMAD_4_5_TRIALPOINTSTATS__

#include <map>
#ifdef _OPENMP
#include <omp.h>
#endif  // _OPENMP

#include "../Algos/Step.hpp"
#include "../Output/OutputQueue.hpp"
#include "../Type/EvalType.hpp"
#include "../Util/Exception.hpp"
#include "../Util/fileutils.hpp"
#include "../Util/utils.hpp"

#include "../nomad_nsbegin.hpp"

/// Class to manage the trial point stats (Search methods and Poll methods)
/**
Counters are available for trial points generated and points that have been evaluated. The counters are available for each registered eval type.
 We have two counters for each eval type: current and total. The current counters are reset at each start of Algorithm and generateTrialPoints of IterationUtils.
 
 */
class  DLL_ALGO_API TrialPointStats
{
private:
    
    const Step* _parentStep;    ///< The parent step of the step having this trial point stats.
    
    /// The registered eval type
    /**
     If a new eval type is added and not yet registered in this vector, we will get an "out of range" exception. Using the "at" function to access the map with an eval type not inserted triggers this exception.
     */
    const std::vector<EvalType> _allEvalType = {EvalType::BB, EvalType::MODEL, EvalType::SURROGATE};
    
    std::map<EvalType, size_t> _nbTotalEvalsDone;
    std::map<EvalType, size_t> _nbCurrentEvalsDone;
    
    std::map<EvalType, size_t> _nbTotalTrialPointsGenerated;
    std::map<EvalType, size_t> _nbCurrentTrialPointsGenerated;
    
    size_t _nbCalls;
    
    void init();
    
    void initializeMap( std::map<EvalType, size_t> & counter );

    
    
public:
    /// Constructor
    /**
     \param parentStep          The Step parent -- \b IN.
     */
    explicit TrialPointStats(const Step* parentStep):
       _parentStep(parentStep)
    {
        init();
    }
    
    void incrementEvalsDone(size_t nb, EvalType  evalType);
    void incrementTrialPointsGenerated(size_t nb, EvalType  evalType);
    void incrementNbCalls() { _nbCalls ++ ;}
    
    size_t getNbEvalsDone(EvalType  evalType, bool totalCount = true) const ;
    size_t getNbTrialPointsGenerated(EvalType  evalType, bool totalCount = true) const ;
    size_t getNbCalls() const { return _nbCalls; }
    
    /**
     Use the CURRENT stats of the given trialPointStats to update this current trialPointStats (CURRENT and TOTAL)
     */
    void updateWithCurrentStats(const  TrialPointStats & trialPointStats );
    
    void resetCurrentStats(); ///< Reset the current stats for the registered eval types. Resets are performed at start of Algorithm and when calling generatedTrialPoints of IterationUtils.
    
    void updateParentStats();
    
    
    
    
    std::string display() const;
    
};
///   Display method stats.
std::ostream& operator<<(std::ostream& os, const TrialPointStats& stats);

/// Get the mesh values from stream
// std::istream& operator>>(std::istream& is, TrialPointStats& stats);


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_TRIALPOINTSTATS__
