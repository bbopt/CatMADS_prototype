#ifndef __NOMAD_4_5_CS__
#define __NOMAD_4_5_CS__

#include "../../Algos/Algorithm.hpp"
#include "../../Algos/AlgoStopReasons.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for Coordinate Search algorithm sampling.
/**
 Generate the trial points using CS and evaluate them. CS is one of the historic ancestor of Mads. CS shares several steps with Mads.
 \todo Complete documentation
 */
class CS: public Algorithm
{

public:
    /// Constructor
    /**
     \param parentStep          The parent of this step -- \b IN.
     \param stopReasons         The stop reasons for MADS -- \b IN.
     \param runParams           The run parameters that control CS -- \b IN.
     \param pbParams            The problem parameters that control CS -- \b IN.
     \param barrierInitializedFromCache  Flag to initialize barrier from cache or not -- \b IN.
     */
    explicit CS(const Step* parentStep,
                std::shared_ptr<AlgoStopReasons<CSStopType>> stopReasons,
                const std::shared_ptr<RunParameters>& runParams,
                const std::shared_ptr<PbParameters>& pbParams,
                bool barrierInitializedFromCache = true )
    : Algorithm(parentStep, stopReasons, runParams, pbParams)
    {
        init(barrierInitializedFromCache);;
    }
    
    /// Helper for hot restart
    void hotRestartOnUserInterrupt() override;
    
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

#endif // __NOMAD_4_5_CS__
