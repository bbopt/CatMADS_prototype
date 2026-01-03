/**
 \file   EvalQueuePoint.hpp
 \brief  Point specifically designed for EvalQueue.
 \author Viviane Rochon Montplaisir
 \date   October 2018
 \see    EvalQueuePoint.cpp
 */

#ifndef __NOMAD_4_5_EVALQUEUEPOINT__
#define __NOMAD_4_5_EVALQUEUEPOINT__

#include "../Eval/EvalPoint.hpp"

#include <functional>   // For std::function

#include "../nomad_nsbegin.hpp"


/**
 *  Elements of the evaluation queue:
 * - evalPoint: Point to eval and its Eval. It is what goes in the cache.
 * - success: result of the comparison of evalPoint's eval with EvaluatorControl's barrier
 */
class EvalQueuePoint : public EvalPoint
{
private:
    const EvalType  _evalType;          ///< EvalType of the evaluator that must evaluate this point (BB, MODEL, SURROGATE)
    SuccessType     _success;           ///< Result of the comparison of evalPoint's eval with barrier
    bool            _relativeSuccess;   ///< Did better than the previous evaluation

    size_t          _k; ///< The number of the iteration that generated this point. For sorting purposes.

public:

    /// Constructor
    /**
     \param evalPoint       The point to eval and its evaluation. It is what goes in the cache.-- \b IN.
     \param evalType        The type of evaluation (BB, MODEL,...).-- \b IN.
     */
    explicit EvalQueuePoint(const EvalPoint& evalPoint, EvalType evalType)
      : EvalPoint(evalPoint),
        _evalType(evalType),
        _success(SuccessType::UNDEFINED),
        _relativeSuccess(false),
        _k(0)
    {}

    const EvalType& getEvalType() const { return _evalType; }

    void setSuccess(const SuccessType success) { _success = success; }
    const SuccessType& getSuccess() const { return _success; }

    void setRelativeSuccess(const bool relativeSuccess) { _relativeSuccess = relativeSuccess; }
    bool getRelativeSuccess() const { return _relativeSuccess; }

    void setK(const size_t k) { _k = k; };
    size_t getK() const { return _k; }
    
    /// Comparison operator \c ==.
    /**
     \param evalQueuePoint   The right-hand side object -- \b IN.
     \return            \c true if  \c *this \c == \c p, \c false if not.
     */
    bool operator== (const EvalQueuePoint& evalQueuePoint) const;

    /// Comparison operator \c !=.
    /**
     \param evalQueuePoint   The right-hand side object -- \b IN.
     \return            \c false if  \c *this \c == \c p, \c true if not.
     */
    bool operator!= (const EvalQueuePoint& evalQueuePoint) const { return !(*this == evalQueuePoint); }
};

/// Smart pointer to EvalQueuePoint
typedef std::shared_ptr<EvalQueuePoint> EvalQueuePointPtr;

/// Block of EvalQueuePointPtrs
typedef std::vector<EvalQueuePointPtr> BlockForEval;


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_EVALQUEUEPOINT__


