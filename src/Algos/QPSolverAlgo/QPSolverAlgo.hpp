#ifndef __NOMAD_4_5_QPSOLVERALGO__
#define __NOMAD_4_5_QPSOLVERALGO__


#include "../../Algos/Algorithm.hpp"
#include "../../Algos/AlgoStopReasons.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class implementing a QP solver algorithm based on quad model.
/**
 Generates trial points by optimizing on a quad model.
 */
class QPSolverAlgo: public Algorithm
{
public:
    /// Constructor
    /**
     \param parentStep          The parent of this Step -- \b IN.
     \param stopReasons        The stop reasons for this algo -- \b IN.
     \param runParams             The run parameters that control this algo -- \b IN.
     \param pbParams               The problem parameters that control the algo -- \b IN.
     */
    explicit QPSolverAlgo(const Step* parentStep,
                std::shared_ptr<AlgoStopReasons<ModelStopType>> stopReasons,
                const std::shared_ptr<RunParameters>& runParams,
                const std::shared_ptr<PbParameters>& pbParams)
      : Algorithm(parentStep, stopReasons, runParams, pbParams)
    {
        init();
    }

    /// Destructor
    virtual ~QPSolverAlgo() {}

    virtual void readInformationForHotRestart() override ;

private:
    /// Helper for constructor
    void init();

    /// Implementation for run tasks.
    /**
     - Algorithm execution for single-objective.
     - Loop on QPSolverOptimize (start, run, end) until a stop reason to terminate is obtained.
     - Update the succes type
     - Perform Termination tasks (start, run, end)
     - Update the SearchMethod success type with best success found.
     \return \c true
     */
    virtual bool runImp() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QPSOLVERALGO__
