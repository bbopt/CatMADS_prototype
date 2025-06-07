#ifndef __NOMAD_4_5_TERMINATION__
#define __NOMAD_4_5_TERMINATION__

#include "../Algos/Step.hpp"

#include "../nomad_nsbegin.hpp"

///  Class for termination of an algorithm.
/**
 The terminate function checks for termination criteria such as MAX_ITERATIONS, MAX_TIME, STOP_IF_FEASIBLE and set the stop reason.
 */
class Termination: public Step
{
private:
    SPAttribute<size_t> _maxIterations, _maxTime;
    SPAttribute<bool> _stopIfFeasible, _stopIfPhaseOneSolution ;
    
public:
    /// Constructor
    explicit Termination(const Step* parentStep,
                         const std::shared_ptr<RunParameters> & runParams ,
                         const std::shared_ptr<PbParameters> & pbParams)
      : Step(parentStep, runParams, pbParams)
    {
        init();
    }

    /// Destructor
    virtual ~Termination() {}

    /**
     The terminate function is called when algorithm are performing iterations during a run. At each iteration, we test if a stop criterion is reached.
     */
    virtual bool terminate(size_t iteration);

    /// No start task is required
    virtual void    startImp() override {}

    /// Implementation for run task of algorithm Termination.
    /**
     \return \c true is a stop reason requires termination of an algorithm, \c false otherwise.
     */
    virtual bool    runImp()   override;

    /// Implementation for end tasks of algorithm Termination.
    /**
     Upon completing an algorithm run, this end function is called to display termination info.
     */
    virtual void    endImp()   override;

private:

    /// Helper for constructor
    void init();
};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_TERMINATION__
