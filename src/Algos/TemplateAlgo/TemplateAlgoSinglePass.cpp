
#include "../../Algos/TemplateAlgo/TemplateAlgoSinglePass.hpp"

#include "../../Algos/SubproblemManager.hpp"

void NOMAD::TemplateAlgoSinglePass::init()
{
    
}

std::string NOMAD::TemplateAlgoSinglePass::getName() const
{
    return "Single pass random algo";
}

void NOMAD::TemplateAlgoSinglePass::startImp()
{
    if ( ! _stopReasons->checkTerminate() )
    {
        // The iteration start function manages the Update .
        TemplateAlgoIteration::startImp();

        generateTrialPoints();
        if (_projectOnMesh && !verifyPointsAreOnMesh(getName()))
        {
            OUTPUT_INFO_START
            AddOutputInfo("At least one trial point is not on mesh. May need investigation if this happens too often.");
            OUTPUT_INFO_END
        }
    }
}


void NOMAD::TemplateAlgoSinglePass::generateTrialPointsImp ()
{
    // Create trial points but no evaluation
    _templateAlgoRandom->start();
    _templateAlgoRandom->end();
    const auto& trialPts = _templateAlgoRandom->getTrialPoints();
    for (auto evalPoint : trialPts)
    {
        evalPoint.addGenStep(getStepType());
        insertTrialPoint(evalPoint);
    }
}
