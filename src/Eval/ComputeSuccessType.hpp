/**
 \file   ComputeSuccessType.hpp
 \brief  Comparison methods for EvalPoints
 \author Viviane Rochon Montplaisir
 \date   April 2017 / September 2020
 \see    ComputeSuccessType.cpp
 */

#ifndef __NOMAD_4_5_COMPUTESUCCESSTYPE__
#define __NOMAD_4_5_COMPUTESUCCESSTYPE__

#include "../Eval/EvalPoint.hpp"

#include "../nomad_nsbegin.hpp"


class DLL_EVAL_API ComputeSuccessType
{
private:
    
    FHComputeType _computeType;
public:

    /// Constructor
    explicit ComputeSuccessType(const FHComputeType & computeType) :
        _computeType(computeType)
    {
    }

    /// Function call operator
    /**
     \param p1      First eval point -- \b IN.
     \param p2      Second eval point -- \b IN.
     \param hMax    Max acceptable infeasibility to keep point in barrier -- \b IN.
     \return        Success type of p1 over p2, considering hMax
     */
    SuccessType operator()(const EvalPointCstPtr & p1,
                           const EvalPointCstPtr & p2,
                           const Double& hMax = INF);


};
#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_COMPUTESUCCESSTYPE__
