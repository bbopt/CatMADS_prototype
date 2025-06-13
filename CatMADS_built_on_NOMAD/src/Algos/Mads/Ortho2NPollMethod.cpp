
#include "../../Algos/Mads/Ortho2NPollMethod.hpp"
#include "../../Math/Direction.hpp"

void NOMAD::Ortho2NPollMethod::init()
{
    setStepType(NOMAD::StepType::POLL_METHOD_ORTHO_2N);
    verifyParentNotNull();

}

// Generate poll directions
void NOMAD::Ortho2NPollMethod::generateUnitPollDirections(std::list<NOMAD::Direction> &directions, const size_t n) const
{
    directions.clear();
    
    generate2NDirections(directions, n);

}
