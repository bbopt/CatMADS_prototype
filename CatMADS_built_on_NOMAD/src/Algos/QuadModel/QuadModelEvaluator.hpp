#ifndef __NOMAD_4_5_QUAD_MODEL_EVALUATION__
#define __NOMAD_4_5_QUAD_MODEL_EVALUATION__

#include "../../Eval/Evaluator.hpp"
#include "../../Output/OutputInfo.hpp"

#include "../../../ext/sgtelib/src/Surrogate.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for evaluating trial points as EvalType::MODEL.
class QuadModelEvaluator : public Evaluator
{
private:
    const std::shared_ptr<SGTELIB::Surrogate> _model;
    std::string                _modelDisplay;
    OutputLevel                _displayLevel;
    Point                      _fixedVariable;  ///< Points maybe sent to evaluator in  full space. Evaluator works in local sub space. In this case this member is used for conversions. Can be undefined: sub=full.

    
    size_t _nbConstraints,_nbModels;
    
    
public:
    /// Constructor
    /**
     Quad model evaluators work in the local full space. No need to pass the fixed variables
     */
    explicit QuadModelEvaluator(const std::shared_ptr<EvalParameters>& evalParams,
                                const std::shared_ptr<SGTELIB::Surrogate>& model,
                                const std::string& modelDisplay,
                                const Point& fixedVariable)
      : Evaluator(evalParams, EvalType::MODEL),
        _model(model),
        _modelDisplay(modelDisplay),
        _displayLevel(OutputLevel::LEVEL_INFO),
        _fixedVariable(fixedVariable)
    {
        init();
    }

    virtual ~QuadModelEvaluator();

    /**
     Points for evaluations are given in a block. Sgtelib models handle the points as a matrix and return a matrix for outputs.
     */
    std::vector<bool> eval_block(Block &block,
                                 const Double &NOMAD_UNUSED(hMax),
                                 std::vector<bool> &countEval) const override;

private:
    void init();


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_EVALUATION__
