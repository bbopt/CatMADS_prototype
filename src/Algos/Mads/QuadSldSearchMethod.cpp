
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/Mads/QuadSldSearchMethod.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Output/OutputQueue.hpp"

//
// Reference: File Quad_Model_Search.cpp in NOMAD 3.9.1
// Author: Sébastien Le Digabel

void NOMAD::QuadSldSearchMethod::init()
{
    setStepType(NOMAD::StepType::SEARCH_METHOD_QUAD_MODEL_SLD);
    verifyParentNotNull();

    const auto parentSearch = getParentStep()->getParentOfType<NOMAD::QuadSldSearchMethod*>(false);

    // For some testing, it is possible that _runParams is null or evaluator control is null
    setEnabled((nullptr == parentSearch)
               && (nullptr !=_runParams)
               && _runParams->getAttributeValue<bool>("QUAD_MODEL_SLD_SEARCH")
               &&  (nullptr != EvcInterface::getEvaluatorControl()));

    // Check that there is exactly one objective
    if (isEnabled())
    {
        auto modelDisplay = _runParams->getAttributeValue<std::string>("QUAD_MODEL_DISPLAY");
        _displayLevel = modelDisplay.empty()
                            ? NOMAD::OutputLevel::LEVEL_DEBUGDEBUG
                            : NOMAD::OutputLevel::LEVEL_INFO;
    }
}


void NOMAD::QuadSldSearchMethod::generateTrialPointsFinal()
{

    // The trial points are generated for a feasible frame center and an infeasible one.
    if ( ! _stopReasons->checkTerminate() )
    {
        auto madsIteration = getParentOfType<MadsIteration*>();

        // MegaIteration's barrier member is already in sub dimension.
        auto bestXFeas = madsIteration->getMegaIterationBarrier()->getCurrentIncumbentFeas();;
        auto bestXInf  = madsIteration->getMegaIterationBarrier()->getCurrentIncumbentInf();;

        
        auto computeType = madsIteration->getMegaIterationBarrier()->getFHComputeType();

        if (nullptr != bestXFeas
            && bestXFeas->getF(computeType).isDefined()
            && bestXFeas->getF(computeType) < MODEL_MAX_OUTPUT)
        {
            NOMAD::QuadModelSldSinglePass singlePassFeas(this, bestXFeas, madsIteration->getMesh());
            
            // Generate the trial points
            singlePassFeas.generateTrialPoints();
            
            // Pass the generated trial pts to this
            const auto& trialPtsSinglePassFeas = singlePassFeas.getTrialPoints();
            for (auto evalPoint : trialPtsSinglePassFeas)
            {
                evalPoint.setPointFrom(bestXFeas, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
                insertTrialPoint(evalPoint);
            }
        }
        if (nullptr != bestXInf
            && bestXInf->getF(computeType).isDefined()
            && bestXInf->getF(computeType) < MODEL_MAX_OUTPUT
            && bestXInf->getH(computeType).isDefined()
            && bestXInf->getH(computeType) < MODEL_MAX_OUTPUT)
        {
            NOMAD::QuadModelSldSinglePass singlePassInf(this, bestXInf, madsIteration->getMesh());

            // Generate the trial points
            singlePassInf.generateTrialPoints();

            // Pass the generated trial pts to this
            const auto& trialPtsSinglePassInf = singlePassInf.getTrialPoints();
            for (auto evalPoint : trialPtsSinglePassInf)
            {
                evalPoint.setPointFrom(bestXInf, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
                insertTrialPoint(evalPoint);
            }
        }
    }

}
 // end generateTrialPoints
