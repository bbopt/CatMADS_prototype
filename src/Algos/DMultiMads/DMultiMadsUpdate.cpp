#include "../../Algos/DMultiMads/DMultiMadsBarrier.hpp"
#include "../../Algos/DMultiMads/DMultiMadsMegaIteration.hpp"
#include "../../Algos/DMultiMads/DMultiMadsUpdate.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/SubproblemManager.hpp"

void NOMAD::DMultiMadsUpdate::init()
{
    setStepType(NOMAD::StepType::UPDATE);
    verifyParentNotNull();
}


std::string NOMAD::DMultiMadsUpdate::getName() const
{
    return getAlgoName() + NOMAD::stepTypeToString(_stepType);
}

bool NOMAD::DMultiMadsUpdate::runImp()
{
    std::string s;
    // Select the current incumbent (See Algo 4)
    // Two cases depending on previous iteration success:
    // - Success (full and partial success): select a possibly new incumbent point.
    // - Failure: decrease the mesh and select the current incumbent point

    // megaIter is already in subproblem.
    // So no need to convert from full dimension to subproblem.
    auto megaIter = getParentOfType<NOMAD::DMultiMadsMegaIteration*>();
    auto iter = getParentOfType<NOMAD::DMultiMadsIteration*>();
    auto barrier = std::dynamic_pointer_cast<NOMAD::DMultiMadsBarrier>(megaIter->getBarrier());

    if (nullptr == barrier)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "DMultiMadsUpdate: No barrier available");
    }
    
    if (NOMAD::EvalType::BB != barrier->getEvalType())
    {
        s = "DMultiMadsUpdate: Only BB eval type is handled";
    }

    if (NOMAD::ComputeType::STANDARD != barrier->getComputeType())
    {
        s = "DMultiMadsUpdate: Only STANDARD compute type is handled";
        throw NOMAD::Exception(__FILE__, __LINE__, s);
    }

    OUTPUT_DEBUG_START
    s = "Running " + getName();
    AddOutputDebug(s);
    OUTPUT_DEBUG_END

    // If the previous iteration is not a success, the meshes of current incumbents are refined (no success). If not, the current incumbents may change.
    NOMAD::SuccessType previousSuccess = iter->getPreviousSuccessType();
    
    // If previous success type is not a success (partial or full), reduce the mesh associated to the current frame center.
    // The frame center can be set if it does not already exist (just after initialization)
    if (previousSuccess < NOMAD::SuccessType::PARTIAL_SUCCESS)
    {
        OUTPUT_DEBUG_START
        s = "Update: previous iter, NO success found";
        AddOutputDebug(s);
        OUTPUT_DEBUG_END
        
        // If the iteration has a frameCenter, it will remain unchanged, except for the mesh.
        if ( nullptr != iter->getFrameCenter())
        {
            iter->getFrameCenter()->getMesh()->refineDeltaFrameSize();
        }
    }
    else
    {
        // No need to update incumbents if mesh of incumbents has not changed.
        // Incumbents have been updated when updated the barrier with eval points
        
        OUTPUT_DEBUG_START
        s = "Update: previous iter, success found";
        AddOutputDebug(s);
        OUTPUT_DEBUG_END
    }

    OUTPUT_DEBUG_START
    s = "Barrier: ";
    AddOutputDebug(s);
    // TODO add a parameter to indicate how many non-dominated points should be displayed
    // with and without their meshes for logs.
    std::vector<std::string> vs = barrier->display(100, true);
    for (const auto & si : vs)
    {
        AddOutputDebug(si);
    }
    OUTPUT_DEBUG_END

    // Before selecting new frame centers, update current incumbents.
    barrier->updateCurrentIncumbents();


    // Select frame center. Can change if iteration is successful or not.
    // Current incumbents may have changed because barrier is updated by adding points or by because the frame size of a barrier point has changed (see above).
    auto currentBestFeas =  barrier->getCurrentIncumbentFeas();
    auto currentBestInf = barrier->getCurrentIncumbentInf();
    
    if (nullptr == currentBestFeas && nullptr == currentBestInf)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Update cannot set iteration frame center (no current best feas or infeas).");
    }
    
    if ( nullptr != currentBestFeas )
    {
        iter->setFrameCenter(currentBestFeas);
    }
    else if ( nullptr != currentBestInf )
    {
        iter->setFrameCenter(currentBestInf);
    }
    OUTPUT_INFO_START
    AddOutputInfo("Frame center: " + iter->getFrameCenter()->display());
    AddOutputInfo("Number of points in Lk (feas+inf): " + std::to_string(barrier->nbXFeas()+barrier->nbXInf()));
    AddOutputInfo("delta mesh size = " + iter->getFrameCenter()->getMesh()->getdeltaMeshSize().display());
    AddOutputInfo("Delta frame size = " + iter->getFrameCenter()->getMesh()->getDeltaFrameSize().display());
    OUTPUT_INFO_END
    
    
    return true;
}
