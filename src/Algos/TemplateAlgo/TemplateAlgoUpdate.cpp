
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoMegaIteration.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoUpdate.hpp"

void NOMAD::TemplateAlgoUpdate::init()
{
    setStepType(NOMAD::StepType::UPDATE);
    verifyParentNotNull();
}


std::string NOMAD::TemplateAlgoUpdate::getName() const
{
    return getAlgoName() + NOMAD::stepTypeToString(_stepType);
}

bool NOMAD::TemplateAlgoUpdate::runImp()
{
    
    // Update the Algo Iteration best point from MegaIteration barrier
    // THIS IS TO ILLUSTRATE AN UPDATE. It could have been done more direct.
    
    bool updateSuccess = false;
    auto barrier = getMegaIterationBarrier();
    auto iter = getParentOfType<NOMAD::TemplateAlgoIteration*>();
    
    if (nullptr == barrier)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Update must have a barrier in the MegaIteration among its ancestors.");
    }
    if (nullptr == iter)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Update must have an Iteration among its ancestors.");
    }
    auto bestXFeas = barrier->getCurrentIncumbentFeas();
    auto bestXInf  = barrier->getCurrentIncumbentInf();
    
    // This is the bestXFeas or bestXInf from MegaIteration barrier.
    // Update them (done at the start of Iteration)
    if (nullptr != bestXFeas)
    {
        iter->setFrameCenter(bestXFeas);
        updateSuccess = true;
    }
    else if (nullptr != bestXInf)
    {
        iter->setFrameCenter(bestXInf);
        updateSuccess = true;
    }
    OUTPUT_DEBUG_START
    auto frameCenter = iter->getFrameCenter();
    AddOutputDebug("Current frame center: " + (frameCenter ? frameCenter->display() : "NULL"));
    OUTPUT_DEBUG_END

    return updateSuccess;
}
