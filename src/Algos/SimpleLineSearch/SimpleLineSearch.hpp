#ifndef __NOMAD_4_5_SIMPLELINESEARCH__
#define __NOMAD_4_5_SIMPLELINESEARCH__


#include "../../Algos/Algorithm.hpp"
#include "../../Algos/AlgoStopReasons.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class implementing a simple line search.
/**
 The simple line search generates trial points from poll centers using direction of success.
  - The first point is generated using a fixed base factor and evaluated.
  - Depending on the success/failure of the first point, a second point is generated. This point is determined to minimize a quadratic model of the objective along the direction of success. The model is built using the poll center and the first point generated.
 */
class SimpleLineSearch: public Algorithm
{
public:
    /// Constructor
    /**
     \param parentStep          The parent of this Step -- \b IN.
     \param stopReasons        The stop reasons for this algo -- \b IN.
     \param runParams             The run parameters that control this algo -- \b IN.
     \param pbParams               The problem parameters that control the algo -- \b IN.
     */
    explicit SimpleLineSearch(const Step* parentStep,
                std::shared_ptr<AlgoStopReasons<SimpleLineSearchStopType>> stopReasons,
                const std::shared_ptr<RunParameters>& runParams,
                const std::shared_ptr<PbParameters>& pbParams)
      : Algorithm(parentStep, stopReasons, runParams, pbParams)
    {
        init();
    }

    /// Destructor
    virtual ~SimpleLineSearch() {}

    virtual void readInformationForHotRestart() override ;

private:
    /// Helper for constructor
    void init();

    /// Implementation for run tasks.
    /**
     - Algorithm execution for single-objective.
     - Update the success type
     - Perform Termination tasks (start, run, end)
     - Update the SearchMethod success type with best success found.
     \return \c true
     */
    virtual bool runImp() override;


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SIMPLELINESEARCH__
