#ifndef __NOMAD_4_5_QUAD_MODEL_SLD_INITIALIZATION__
#define __NOMAD_4_5_QUAD_MODEL_SLD_INITIALIZATION__

#include "../../Algos/Initialization.hpp"
#include "../../Algos/IterationUtils.hpp"

#include "../../nomad_nsbegin.hpp"


class QuadModelSldInitialization: public Initialization, public IterationUtils
{
private:
    std::shared_ptr<AlgoStopReasons<ModelStopType>> _qmStopReason;
public:
    /// Constructor
    explicit QuadModelSldInitialization(const Step* parentStep)
      : Initialization(parentStep),
        IterationUtils(parentStep)
    {
        init();
    }

    virtual ~QuadModelSldInitialization();


private:
    void init();

    virtual void startImp() override;
    virtual bool runImp() override;
    void endImp() override {};

    /// Insert X0s for evaluation or (exclusive) check cache
    void generateTrialPointsImp() override;

    /// Eval X0s, using blackbox.
    bool eval_x0s();

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_SLD_INITIALIZATION__

