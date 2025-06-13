
#ifndef __NOMAD_4_5_SGTELIB_MODEL_EVALUATION__
#define __NOMAD_4_5_SGTELIB_MODEL_EVALUATION__

#include "../../Algos/SgtelibModel/SgtelibModel.hpp"
#include "../../Eval/Evaluator.hpp"
#include "../../Type/SgtelibModelFeasibilityType.hpp"
#include "../../Output/OutputInfo.hpp"  // for OutputLevel

#include "../../nomad_nsbegin.hpp"

class SgtelibModelEvaluator : public Evaluator
{
private:
    const SgtelibModel*         _modelAlgo;
    std::string                 _modelDisplay;
    Double                      _diversification;
    SgtelibModelFeasibilityType _modelFeasibility;
    double                      _tc;
    OutputLevel                 _displayLevel;
    Point                       _fixedVariable;

public:
    /// Constructor
    explicit SgtelibModelEvaluator(const std::shared_ptr<EvalParameters>& evalParams,
                                   const SgtelibModel* modelAlgo,
                                   const std::string& modelDisplay,
                                   const Double& diversification,
                                   const SgtelibModelFeasibilityType& modelFeasibility,
                                   const double tc,
                                   const Point& fixedVariable);

    virtual ~SgtelibModelEvaluator();

    bool eval_x(EvalPoint &x,
                const Double &NOMAD_UNUSED(hMax),
                bool &countEval) const override;

private:
    void init();

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SGTELIB_MODEL_EVALUATION__
