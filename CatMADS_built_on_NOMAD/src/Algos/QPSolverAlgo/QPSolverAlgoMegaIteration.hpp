#ifndef __NOMAD_4_5_QPSOLVERALGOMEGAITERATION__
#define __NOMAD_4_5_QPSOLVERALGOMEGAITERATION__

#include "../../Algos/MegaIteration.hpp"
#include "../../Algos/QPSolverAlgo/QPSolverAlgoIteration.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for QPSolverAlgo mega iteration.
/**
 * Manager for QPSolverAlgo iterations starts, runs and ends.
 * Steps:
 * - Generate points over simplices (start).
 * - Evaluate points (run)
 * - Post-processing (end)
 */
class QPSolverAlgoMegaIteration: public MegaIteration
{
private:
    std::vector<std::shared_ptr<QPSolverAlgoIteration>> _iterList;

public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param k               The main iteration counter -- \b IN.
     \param barrier         The barrier for constraints handling -- \b IN.
     \param success         Success type of the previous MegaIteration. -- \b IN.
     */
    explicit QPSolverAlgoMegaIteration(const Step* parentStep,
                              size_t k,
                              std::shared_ptr<BarrierBase> barrier,
                              SuccessType success)
    : MegaIteration(parentStep, k,barrier,success)
    {
        init();
    }
    // No Destructor needed - keep defaults.


    void read(  std::istream& is ) override;
    void display(  std::ostream& os ) const override ;

private:

    void init();

    /// Implementation of start task.
    /**
     \note Running the algorithm requires a single iteration object with several start, run, end for the various iterations of the algorithm.
     */
    virtual void startImp() override ;

    /// Implementation of run task.
    /**
     The algorithm iterations are started, ran and ended sequentially until a stop reason to terminate is obtained. \n
     We have a success if either a better xFeas or
     a dominating or partial success for xInf was found.
     See Algorithm 12.2 from DFBO.
     */
    virtual bool runImp() override;

    // Used in QuadModel but not in Template
    // virtual bool endImp() override;
    

};

/**
 Display useful values so that a new MegaIteration could be constructed using these values.
 */
std::ostream& operator<<(std::ostream& os, const QPSolverAlgoMegaIteration& megaIteration);

/// Get an MegaIteration values from a stream
std::istream& operator>>(std::istream& is, QPSolverAlgoMegaIteration& megaIteration);

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QPSOLVERALGOMEGAITERATION__
