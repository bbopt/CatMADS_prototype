
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/PSDMads/PSDMadsMegaIteration.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Type/DirectionType.hpp"
#include "../../Type/EvalSortType.hpp"
#include "../../Type/LHSearchType.hpp"


void NOMAD::PSDMadsMegaIteration::destroy()
{
    _madsOnSubPb.reset();
    setStepType(NOMAD::StepType::MEGA_ITERATION);
}


void NOMAD::PSDMadsMegaIteration::startImp()
{
    auto madsStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::MadsStopType>>();
    bool isPollster = (0 == NOMAD::getThreadNum());

    // Set parameters for subproblems
    auto subProblemPbParams = std::make_shared<NOMAD::PbParameters>(*_pbParams);
    auto subProblemRunParams = std::make_shared<NOMAD::RunParameters>(*_runParams);
    setupSubproblemParams(subProblemPbParams, subProblemRunParams, isPollster);

    // Create Mads for this subproblem
    // The barrier of the algo will be initialized with the Cache.
    _madsOnSubPb = std::make_shared<NOMAD::Mads>(this, madsStopReasons, subProblemRunParams, subProblemPbParams, true /* true: barrier initialized from cache */);
    /*
    std::string madsName = "Mads ";
    if (isPollster)
    {
        madsName += "pollster";
    }
    else
    {
        if (_fixedVariable.size() <= 10)
        {
            madsName += "with fixed variable ";
            madsName += _fixedVariable.display();
        }
        else
        {
            madsName += "with ";
            madsName += NOMAD::itos(_fixedVariable.size() - _fixedVariable.nbDefined());
            madsName += " fixed variables";
        }
    }
    _madsOnSubPb->setName(madsName);
    */
    _madsOnSubPb->setStepType(NOMAD::StepType::ALGORITHM_PSD_MADS_SUBPROBLEM);
    
    
    // Default mega iteration start tasks
    // See issue #639
    NOMAD::MegaIteration::startImp();
}



bool NOMAD::PSDMadsMegaIteration::runImp()
{
    // Run Mads
    // Note: Pollster is always run whenever thread 0 is available.
    // However, contrary to the NOMAD 3 version, mesh is not updated at each pollster run.

    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    int mainThreadNum = NOMAD::getThreadNum();

    OUTPUT_INFO_START
    std::string s = "Running " + _madsOnSubPb->getName();
    s += " on thread " + NOMAD::itos(mainThreadNum);
    AddOutputInfo(s);
    OUTPUT_INFO_END
    
    
    _madsOnSubPb->start();
    bool madsSuccessful = _madsOnSubPb->run();   // If this run is successful, barrier will be updated.
    _madsOnSubPb->end();

    OUTPUT_INFO_START
    std::string s = "Done running " + _madsOnSubPb->getName();
    s += " on thread " + NOMAD::itos(mainThreadNum) + ". ";
    s += "Number of evaluations: " + NOMAD::itos(evc->getBbEvalInSubproblem()) + ". ";
    s += "Found a new success: " + NOMAD::boolToString(madsSuccessful) + ".";
    AddOutputInfo(s);
    OUTPUT_INFO_END
    
    evc->resetBbEvalInSubproblem();

    return madsSuccessful;
}


void NOMAD::PSDMadsMegaIteration::setupSubproblemParams(std::shared_ptr<NOMAD::PbParameters> &subProblemPbParams,
                                           std::shared_ptr<NOMAD::RunParameters> &subProblemRunParams,
                                           const bool isPollster)
{
    auto mainFrameSize = _mainMesh->getDeltaFrameSize();
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();

    // Note: If n >= 50, models are disabled. They could be re-enabled on
    // subproblems with lesser dimension. See issue #370.

    subProblemPbParams->doNotShowWarnings();
    if (isPollster)
    {
        subProblemRunParams->setAttributeValue("DIRECTION_TYPE", NOMAD::DirectionType::SINGLE);
        subProblemPbParams->setAttributeValue("INITIAL_FRAME_SIZE", mainFrameSize);

        // Disable all searches
        subProblemRunParams->setAttributeValue("LH_SEARCH", NOMAD::LHSearchType("0 0"));

        subProblemRunParams->setAttributeValue("NM_SEARCH", false);
        subProblemRunParams->setAttributeValue("QUAD_MODEL_SEARCH", false);
        subProblemRunParams->setAttributeValue("SGTELIB_MODEL_SEARCH", false);
        subProblemRunParams->setAttributeValue("SPECULATIVE_SEARCH", false);
        subProblemRunParams->setAttributeValue("VNS_MADS_SEARCH", false);  // VNS has static member. Problematic with threads. See issue # 604
        
    }
    else
    {
        auto initialFrameSize = _mainMesh->getDeltaFrameSizeCoarser();
        subProblemPbParams->setAttributeValue("INITIAL_FRAME_SIZE", initialFrameSize);

        // The main frame size is used as minFrameSize for the subproblem.
        // Initial and min must be compatible -> adjust.
        for (size_t i = 0; i < initialFrameSize.size(); i++)
        {
            if (initialFrameSize[i].todouble() < mainFrameSize[i].todouble())
            {
                OUTPUT_INFO_START
                AddOutputInfo("Set initial frame size to main frame size.");
                OUTPUT_INFO_END
                subProblemPbParams->setAttributeValue("INITIAL_FRAME_SIZE", mainFrameSize);
                break;
            }
        }

        // Issue #685. Force some algo settings. Need to test more thoroughly what give the best results
        subProblemRunParams->setAttributeValue("NM_SEARCH", false);
        subProblemRunParams->setAttributeValue("QUAD_MODEL_SEARCH", false);
        
        // TODO make it work with ORTHO N+1 QUAD. For now, ORTHO N+1 QUAD seems to have trouble when generating the n+1 th point with quad model (fixed variable pb).
        subProblemRunParams->setAttributeValue("DIRECTION_TYPE",NOMAD::DirectionType::ORTHO_2N);
        
        subProblemPbParams->setAttributeValue("FIXED_VARIABLE", _fixedVariable);
        subProblemPbParams->setAttributeValue("X0", _x0);
        subProblemPbParams->setAttributeValue("MIN_FRAME_SIZE", mainFrameSize);
        subProblemRunParams->setAttributeValue("PSD_MADS_NB_VAR_IN_SUBPROBLEM", _fixedVariable.size() - _fixedVariable.nbDefined());
    }

    // Set max number of bb evals per subproblem.
    // Strategy to be refined.
    if (isPollster)
    {
        evc->setMaxBbEvalInSubproblem(1);
    }
    else
    {
        size_t totalBudget = evc->getEvaluatorControlGlobalParams()->getAttributeValue<size_t>("MAX_BB_EVAL");
        size_t nbSubproblem = _runParams->getAttributeValue<size_t>("PSD_MADS_NB_SUBPROBLEM");
        size_t budgetPerSubproblem = (totalBudget-1) / nbSubproblem;
        size_t maxBbEvalInSubproblem = evc->getMaxBbEvalInSubproblem();
        if (budgetPerSubproblem < maxBbEvalInSubproblem)
        {
            maxBbEvalInSubproblem = budgetPerSubproblem;
        }
        evc->setMaxBbEvalInSubproblem(maxBbEvalInSubproblem);
    }

    subProblemPbParams->checkAndComply();
    auto evcParams = evc->getEvaluatorControlGlobalParams();
    subProblemRunParams->checkAndComply(evcParams, subProblemPbParams);
}
