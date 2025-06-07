#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/CoordinateSearch/CSPoll.hpp"
#include "../../Algos/CoordinateSearch/CSPollMethod.hpp"

void NOMAD::CSPoll::init()
{
    setStepType(NOMAD::StepType::CS_POLL);
}

void NOMAD::CSPoll::startImp()
{
    // May need some refactoring. This code is similar to Poll::startImp().
    
    // Sanity check.
    verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, false);

    // Reset the current counters. The total counters are not reset (done only once when constructor is called).
    _trialPointStats.resetCurrentStats();
    
    
    // Generate CS poll methods
    NOMAD::DirectionTypeList dirTypes = _runParams->getAttributeValue<DirectionTypeList>("DIRECTION_TYPE");
    
    if (dirTypes.size() != 1 || dirTypes[0] != DirectionType::CS)
    {
        throw NOMAD::Exception(__FILE__, __LINE__,"CS Poll method only support DirectionType::CS. " + directionTypeToString(dirTypes[0]) + " not supported for CS.");
    }
    
    // Compute primary and secondary poll centers
    std::vector<NOMAD::EvalPointPtr> primaryCenters, secondaryCenters;
    computePrimarySecondaryPollCenters(primaryCenters, secondaryCenters);

    // Add poll methods for primary polls
    _pollMethods.clear();
    _frameCenters.clear();
    for (const auto & pollCenter : primaryCenters)
    {
        createPollMethods(true, pollCenter);
    }
    // Add poll methods for secondary polls
    for (const auto & pollCenter : secondaryCenters)
    {
        createPollMethods(false, pollCenter);
    }
    
}

void NOMAD::CSPoll::createPollMethods(const bool isPrimary, const EvalPointPtr& frameCenter)
{
    _frameCenters.push_back(frameCenter);
    auto pollMethod = std::make_shared<NOMAD::CSPollMethod>(this, frameCenter);
    _pollMethods.push_back(pollMethod);
}


void NOMAD::CSPoll::setMeshPrecisionStopType()
{
    auto csStopReasons = NOMAD::AlgoStopReasons<NOMAD::CSStopType>::get(_stopReasons);
    csStopReasons->set(NOMAD::CSStopType::MESH_PREC_REACHED);
}
