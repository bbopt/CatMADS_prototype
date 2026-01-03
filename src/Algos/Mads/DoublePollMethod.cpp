
#include "../../Algos/Mads/DoublePollMethod.hpp"
#include "../../Math/Direction.hpp"

void NOMAD::DoublePollMethod::init()
{
    setStepType(NOMAD::StepType::POLL_METHOD_DOUBLE);
    verifyParentNotNull();
}

// Generate poll directions
void NOMAD::DoublePollMethod::generateUnitPollDirections(std::list<NOMAD::Direction> &directions, const size_t n) const
{
    directions.clear();

    NOMAD::Direction dirUnit(n, 0.0);
    NOMAD::Direction::computeDirOnUnitSphere(dirUnit, _rng);
    directions.push_back(dirUnit);

    // insert the opposite direction
    dirUnit *=-1.0;
    directions.push_back(dirUnit);
}
