
#ifndef __NOMAD_4_5_SIMPLEMADS__
#define __NOMAD_4_5_SIMPLEMADS__

#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/QuadModel/QuadModelEvaluator.hpp"
#include "../../Algos/SimpleMads/SimplePoll.hpp"
#include "../../Type/BBInputType.hpp"
#include "../../Type/ComputeType.hpp"

#include "../../nomad_nsbegin.hpp"


/// The (M)esh (A)daptive (D)irect (S)earch algorithm. Simple version for quad model search
/**
 */
class SimpleMads: public Mads
{
private:

    SimplePoll _poll;

    BBOutputTypeList _bbot;

    const size_t _maxEval;

public:
    /// Constructor #1 for model optim
    /**
     \param parentStep                  The parent of this step -- \b IN.
     \param stopReasons                The stop reasons for MADS -- \b IN.
     \param runParams                    The run parameters that control MADS -- \b IN.
     \param pbParams                      The problem parameters that control MADS -- \b IN.
     \param model                             The quadratic model-- \b IN.
     \param bbot                               The bb output type -- \b IN.
     \param singleObjCompute     The function type to compute objective (can be undefined; used by DMultiMads) -- \b IN.
     \param maxEval                         Max evaluation stopping criterion -- \b IN.
     */
    explicit SimpleMads(const Step* parentStep,
                  std::shared_ptr<AlgoStopReasons<MadsStopType>> stopReasons,
                  const std::shared_ptr<RunParameters>& runParams,
                  const std::shared_ptr<PbParameters>& pbParams,
                  const std::shared_ptr<SGTELIB::Surrogate> & model,
                  const BBOutputTypeList & bbot,
                  const singleOutputComputeFType & singleObjCompute,
                  const size_t maxEval )
      : Mads(parentStep, stopReasons, runParams, pbParams, false /* false: barrier not initialized from cache */, false /* false: do not use local fixed variables */),
        _poll(this, model, bbot, singleObjCompute),
        _bbot(bbot),
        _maxEval(maxEval)
    {
        init();
    }

    /// Constructor #2 for model optim
    /**
     \param parentStep                  The parent of this step -- \b IN.
     \param stopReasons                The stop reasons for MADS -- \b IN.
     \param runParams                    The run parameters that control MADS -- \b IN.
     \param pbParams                      The problem parameters that control MADS -- \b IN.
     \param bbot                               The bb output type -- \b IN.
     \param eval_x                           The function to compute outputs -- \b IN.
     \param maxEval                         Max evaluation stopping criterion -- \b IN.
     */
    explicit SimpleMads(const Step* parentStep,
                  std::shared_ptr<AlgoStopReasons<MadsStopType>> stopReasons,
                  const std::shared_ptr<RunParameters>& runParams,
                  const std::shared_ptr<PbParameters>& pbParams,
                  const BBOutputTypeList & bbot,
                  std::function<bool(std::vector<NOMAD::SimpleEvalPoint>&)> eval_x,
                  const size_t maxEval )
      : Mads(parentStep, stopReasons, runParams, pbParams, false /* false: barrier not initialized from cache */, false /* false: do not use local fixed variables */),
        _poll(this, bbot, eval_x),
        _bbot(bbot),
        _maxEval(maxEval)
    {
        init();
    }

    const SimpleEvalPoint & getBestSimpleSolution(bool bestFeas) const ;

    EvalPoint getBestSolution (bool bestFeas) const override;

private:
    ///  Initialization of class, to be used by Constructor.
    void init();

    virtual void startImp() override {};

    /// Algorithm execution for single-objective.
    /**
     Overrides the default algorithm's run
     \return \c true if a full success was found, \c false otherwise
     */
    virtual bool runImp() override;

    virtual void endImp() override;

    void endDisplay() const ;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SIMPLEMADS__
