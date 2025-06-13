
#include "../../Algos/NelderMead/NMUpdate.hpp"

void NOMAD::NMUpdate::init()
{
    setStepType(NOMAD::StepType::UPDATE);
    verifyParentNotNull();
}

std::string NOMAD::NMUpdate::getName() const
{
    return getAlgoName() + NOMAD::stepTypeToString(_stepType);
}
