
#include "../../Algos/Mads/LHSearchMethod.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Math/LHS.hpp"
#include "../../Type/LHSearchType.hpp"

void NOMAD::LHSearchMethod::init()
{
    setStepType(NOMAD::StepType::SEARCH_METHOD_LH);

    // For some testing, it is possible that _runParams is null
    if ( nullptr != _runParams)
    {
        auto lhSearch = _runParams->getAttributeValue<NOMAD::LHSearchType>("LH_SEARCH");
        setEnabled(lhSearch.isEnabled());
    }
    else
    {
        setEnabled(false);
    }
}


void NOMAD::LHSearchMethod::generateTrialPointsFinal()
{
    if (nullptr == _iterAncestor)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"LHSearchMethod: must have an iteration ancestor");
    }
    auto mesh = _iterAncestor->getMesh();
    if (nullptr == mesh)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"LHSearchMethod: must have a mesh");
    }

    // The frame center is only used to compute bounds, if they are not defined.
    // Use the first available point.
    auto barrier = getMegaIterationBarrier();
    if (nullptr == barrier)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"LHSearchMethod: must have a MadsMegaIteration ancestor with a barrier");
    }
    auto frameCenter = barrier->getFirstPoint();

    auto lhSearch = _runParams->getAttributeValue<NOMAD::LHSearchType>("LH_SEARCH");
    size_t n = _pbParams->getAttributeValue<size_t>("DIMENSION");
    size_t p = (0 == _iterAncestor->getK()) ? lhSearch.getNbInitial() : lhSearch.getNbIteration();
    auto lowerBound = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND");
    auto upperBound = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND");

    NOMAD::ArrayOfDouble deltaFrameSize = mesh->getDeltaFrameSize();
    NOMAD::Double scaleFactor = sqrt(-log(NOMAD::DEFAULT_EPSILON));
    // Apply Latin Hypercube algorithm (provide frameCenter, deltaFrameSize, and scaleFactor for updating bounds)
    NOMAD::LHS lhs(n, p, lowerBound, upperBound, *frameCenter, deltaFrameSize, scaleFactor);
    auto pointVector = lhs.Sample();

    // Insert the point. Projection on mesh and snap to bounds is done in SearchMethod
    for (const auto & point : pointVector)
    {
        // Insert point (if possible)
        NOMAD::EvalPoint evalPoint(point);
        evalPoint.setPointFrom(std::make_shared<NOMAD::EvalPoint>(*frameCenter), NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this)); // !!! Point from is a copy of frame center
        evalPoint.addGenStep(getStepType());
        insertTrialPoint(evalPoint);
    }
}
