#include "../../Algos/Mads/UserSearchMethod.hpp"

#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/SubproblemManager.hpp"

void NOMAD::UserSearchMethod::init()
{
    setStepType(NOMAD::StepType::SEARCH_METHOD_USER);

    // Query the enabling parameter here
    if (nullptr == _iterAncestor)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"UserSearchMethod (" + std::to_string(_id) +"): must have an iteration ancestor");
    }

    if (_id > 2 || _id <= 0)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"UserSearchMethod (" + std::to_string(_id) +"): only id =1 and id =2 are supported");
    }
    auto mads = dynamic_cast<const NOMAD::Mads*>(_iterAncestor->getRootAlgorithm());
    if ( nullptr != mads)
    {
        setEnabled(mads->hasUserSearchMethod());
    }
    else
    {
        setEnabled(false);
    }
}


void NOMAD::UserSearchMethod::generateTrialPointsFinal()
{

    // The frame center is only used to compute bounds, if they are not defined.
    // Use the first available point.
    auto barrier = getMegaIterationBarrier();
    if (nullptr == barrier)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"UserSearchMethod (" + std::to_string(_id) +"): must have a MadsMegaIteration ancestor with a barrier");
    }
    auto frameCenter = barrier->getFirstPoint();

    OUTPUT_INFO_START
    AddOutputInfo("Generate point for " + getName() + "(" + std::to_string(_id) +")");
    OUTPUT_INFO_END

    auto mads = dynamic_cast<const NOMAD::Mads*>(_iterAncestor->getRootAlgorithm());


    bool success =false;
    if (_id == 1)
    {
        success = mads->runCallback(NOMAD::CallbackType::USER_METHOD_SEARCH, *this, _trialPoints);
    }
    else if (_id == 2)
    {
        success = mads->runCallback(NOMAD::CallbackType::USER_METHOD_SEARCH_2, *this, _trialPoints);
    }

    if (!success)
    {
        OUTPUT_INFO_START
        AddOutputInfo("User search (" + std::to_string(_id) +") cannot produce directions.");
        OUTPUT_INFO_END
        return;
    }

    // Insert the point. Projection on mesh and snap to bounds is done later
    for (auto tp : _trialPoints)
    {
        tp.setPointFrom(std::make_shared<NOMAD::EvalPoint>(*frameCenter), NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this)); // !!! Point from is a copy of frame center
        tp.addGenStep(getStepType());
        insertTrialPoint(tp);
    }
}

void NOMAD::UserSearchMethod::updateAtStepEnd()
{
    auto mads = dynamic_cast<const NOMAD::Mads*>(_iterAncestor->getRootAlgorithm());

    bool success = mads->runCallback(NOMAD::CallbackType::USER_METHOD_SEARCH_END, *this);

    if (!success)
    {
        OUTPUT_INFO_START
        AddOutputInfo("User search (" + std::to_string(_id) +") post evaluation is not working properly.");
        OUTPUT_INFO_END
        return;
    }
}
