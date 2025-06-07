/**
 \file   DiscoMadsUpdate.hpp
 \brief  The DiscoMads algorithm update step
 \author Solene Kojtych
 \see    DiscoMadsUpdate.cpp
 */
#include "../../Algos/CacheInterface.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/DiscoMads/DiscoMadsMegaIteration.hpp"
#include "../../Algos/DiscoMads/DiscoMadsUpdate.hpp"
#include "../../Algos/Mads/MadsUpdate.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Eval/ComputeSuccessType.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::DiscoMadsUpdate::init()
{
    setStepType(NOMAD::StepType::UPDATE);
    verifyParentNotNull();

    auto megaIter = getParentOfType<NOMAD::DiscoMadsMegaIteration*>();
    if (nullptr == megaIter)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Error: An instance of class DiscoMadsUpdate must have a DiscoMegaIteration among its ancestors");
    }
    
    _clearEvalQueue = true;
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    if (nullptr != evc)
    {
        _clearEvalQueue = evc->getEvaluatorControlGlobalParams()->getAttributeValue<bool>("EVAL_QUEUE_CLEAR");
    }

}




bool NOMAD::DiscoMadsUpdate::runImp()
{
    bool lastIterationRevealing = getParentOfType<NOMAD::DiscoMadsMegaIteration*>()->isRevealing();

    // Number corresponding to current iteration
    size_t numIter = getParentOfType<NOMAD::DiscoMadsMegaIteration*>()->getK();

    // If last iteration was revealing, special update 
    if (lastIterationRevealing){
        // Barrier is already updated from previous steps (IterationUtils::postProcessing).
    
        // megaIter barrier is already in subproblem.
        // So no need to convert refBestFeas and refBestInf
        // from full dimension to subproblem.
        auto megaIter = getParentOfType<NOMAD::MadsMegaIteration*>();
        auto barrier = megaIter->getBarrier();
        auto mesh = megaIter->getMesh();
        std::string s;  // for output

        //Step 1. Get the best feasible and infeasible reference points, and then update reference values.
        auto refBestFeas = barrier->getRefBestFeas();
        auto refBestInf  = barrier->getRefBestInf();
        barrier->updateRefBests();

        // Display of current barrier at the beginning of the iteration considering updated ref points
        OUTPUT_DEBUG_START
        s = "Running " + getName() + ". Barrier: ";
        AddOutputDebug(s);
        std::vector<std::string> vs = barrier->display(4);
        for (const auto & si : vs)
        {
            AddOutputDebug(si);
        }
        s = "Update: revealing iteration";
        AddOutputDebug(s);
        OUTPUT_DEBUG_END

        //Step 2. Update mesh size parameter and frame parameter
        s = "Last Iteration revealing (iteration " + std::to_string(numIter-1) +"). Delta remains the same.";
        AddOutputInfo(s);
        mesh->updatedeltaMeshSize();   // update by security 

        OUTPUT_INFO_START
        AddOutputInfo("delta mesh size = " + mesh->getdeltaMeshSize().display());
        AddOutputInfo("Delta frame size = " + mesh->getDeltaFrameSize().display());
        OUTPUT_INFO_END

        //Step 3. Reset revealing status of  megaIteration in  DiscoMadsMegaIteration::RunImp

    }
    // If las iteration was not revealing, usual update of Mads
    else{
        MadsUpdate::runImp();
    }

    return true;
}


