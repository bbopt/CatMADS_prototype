
#include "../../Algos/Mads/SinglePollMethod.hpp"
#include "../../Math/Direction.hpp"

void NOMAD::SinglePollMethod::init()
{
    setStepType(NOMAD::StepType::POLL_METHOD_SINGLE);
    verifyParentNotNull();
}

// Generate a poll direction
void NOMAD::SinglePollMethod::generateUnitPollDirections(std::list<NOMAD::Direction> &directions, const size_t n) const
{
    NOMAD::Direction dirUnit(n, 0.0);
    NOMAD::Direction::computeDirOnUnitSphere(dirUnit, _rng);
    directions.push_back(dirUnit);
}

