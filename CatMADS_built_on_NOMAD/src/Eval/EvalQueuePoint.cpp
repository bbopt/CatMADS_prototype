#include "../Eval/EvalQueuePoint.hpp"

/*------------------------*/
/* Class EvalQueuePoint   */
/*------------------------*/

bool NOMAD::EvalQueuePoint::operator== (const NOMAD::EvalQueuePoint &evalQueuePoint) const
{
    // First compare eval type.
    if ( this->_evalType != evalQueuePoint.getEvalType() )
        return false;
    
    // Compare EvalPoint part.
    bool equal = NOMAD::EvalPoint::operator==(evalQueuePoint);
    
    return equal;
    
}

