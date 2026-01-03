#include "../../Algos/CoordinateSearch/CSPollMethod.hpp"
#include "../../Math/Direction.hpp"

void NOMAD::CSPollMethod::init()
{
    setStepType(NOMAD::StepType::CS_POLL_METHOD);
    verifyParentNotNull();
}

void NOMAD::CSPollMethod::generateUnitPollDirections(std::list<NOMAD::Direction> &directions, const size_t n) const
{
    directions.clear();
    NOMAD::Direction dirUnit(n , 0.0);
    
     /// We want to have unitary direction for each coordinate, hence 1 in the specific coordinate, and 0 for all of the others
    /// We can all positive directions and than all negative (lexicographical order)
    for ( size_t i = 0 ; i < n ; ++i )
    {
        dirUnit[i] = 1.0;
        directions.push_back(dirUnit);
        dirUnit[i] = 0.0;
    }
    for ( size_t i = 0 ; i < n ; ++i )
    {
        dirUnit[i] = -1.0;
        directions.push_back(dirUnit);
        dirUnit[i] = 0.0;
    }
    // Alternatively, we can alternate positive and negative directions for all variables.
//    for ( size_t i = 0 ; i < n ; ++i )
//    {
//        dirUnit[i] = 1.0;
//        directions.push_back(dirUnit);
//        dirUnit[i] = -1.0;
//        directions.push_back(dirUnit);
//        dirUnit[i] = 0.0;
//    }
}
