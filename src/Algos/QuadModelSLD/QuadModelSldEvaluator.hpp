#ifndef __NOMAD_4_5_QUAD_MODEL_SLD_EVALUATION__
#define __NOMAD_4_5_QUAD_MODEL_SLD_EVALUATION__

#include "../../Eval/Evaluator.hpp"
#include "../../Output/OutputInfo.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSld.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for evaluating trial points as EvalType::MODEL.
class QuadModelSldEvaluator : public Evaluator
{
private:
    const std::shared_ptr<QuadModelSld> _model;
    std::string                _modelDisplay;
    OutputLevel                _displayLevel;
    
    int       _n;           ///< Number of variables.
    int       _nm1;         ///< Number of variables minus one.
    int       _m;           ///< Number of blackbox outputs.
    double  * _x;           ///< An evaluation point.
    double ** _alpha;       ///< Model parameters.
    bool      _model_ready; ///< \c true if model ready to evaluate.
    

public:
    /// Constructor
    /**
     Quad model evaluators work in the local full space. No need to pass the fixed variables
     */
    explicit QuadModelSldEvaluator(const std::shared_ptr<EvalParameters>& evalParams,
                                const std::shared_ptr<QuadModelSld>& model,
                                const std::string& modelDisplay)
      : Evaluator(evalParams, EvalType::MODEL),
        _model(model),
        _modelDisplay(modelDisplay),
        _displayLevel(OutputLevel::LEVEL_INFO),
        _n                  ( model->get_n()         ) ,
        _nm1                ( _n-1                  ) ,
        _m                  ( 0                     ) ,
        _x                  ( NULL                  ) ,
        _alpha              ( NULL                  ) ,
        _model_ready        ( model->check()         )
    {
        init();
    }

    virtual ~QuadModelSldEvaluator();

    /**
        Evaluation of given eval point
     */
    bool eval_x(EvalPoint &x,
                const Double& hMax,
                bool &countEval) const override;

private:
    void init();


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_SLD_EVALUATION__
