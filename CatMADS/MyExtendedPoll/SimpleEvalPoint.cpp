/**
 \file   SimpleEvalPoint.cpp
 \brief  Evaluation point for Simple Mads
 \author Sebastien Le Digabel, Viviane Rochon Montplaisir and Christophe Tribes
 \date   March 2017- March 2024
 \see    EvalPoint.hpp
 */
#include "../SimpleMads/SimpleEvalPoint.hpp"
#include "../../Output/OutputQueue.hpp"


///*-----------------------------------------------------------*/
///*                     Affectation operator                  */
///*-----------------------------------------------------------*/
NOMAD::SimpleEvalPoint & NOMAD::SimpleEvalPoint::operator=(const NOMAD::SimpleEvalPoint &evalPoint)
{
    if (this == &evalPoint)
    {
        return *this;
    }

    NOMAD::Point::operator=(evalPoint);
    
    _f = evalPoint.getF();
    _h = evalPoint.getH();

    return *this;
}

bool NOMAD::SimpleEvalPoint::isDefined() const
{
    return ( _f.isDefined() && _h.isDefined() && NOMAD::Point::isDefined());
}


