
#include <algorithm>    // For std::merge and std::unique

#include "../../nomad_platform.hpp"
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoIteration.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoMegaIteration.hpp"

void NOMAD::TemplateAlgoIteration::init()
{
    setStepType(NOMAD::StepType::ITERATION);
    
    _templateAlgoRandom = std::make_unique<NOMAD::TemplateAlgoRandom>(this);
    _templateAlgoUpdate = std::make_unique<NOMAD::TemplateAlgoUpdate> (this);

}

void NOMAD::TemplateAlgoIteration::startImp()
{
    // For illustration purpose the Update is used to update the center point
    // (the best feasible or best infeasible) around which the trial points are generated.
    _templateAlgoUpdate->start();
    bool updateSuccess = _templateAlgoUpdate->run();
    _templateAlgoUpdate->end();
    
    if ( ! updateSuccess )
    {
        auto stopReason = NOMAD::AlgoStopReasons<NOMAD::RandomAlgoStopType>::get ( getAllStopReasons() );

        // The update is not a success. If the global stop reason is not set to terminate we set a default stop reason for initialization.
        if ( !_stopReasons->checkTerminate() )
            stopReason->set( NOMAD::RandomAlgoStopType::UPDATE_FAILED);
    }
}


bool NOMAD::TemplateAlgoIteration::runImp()
{
    // Iteration cannot generate all points before evaluation
    verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, false);

    bool iterationSuccess = false;
    
    
    _templateAlgoRandom->start();
    
    // Iteration is a success if either a better xFeas or
    // a better xInf (partial success or dominating) xInf was found.
    iterationSuccess = _templateAlgoRandom->run();

    _templateAlgoRandom->end();
    
    if ( iterationSuccess )
    {
        // Update the MegaIteration best success type with success found.
        getParentOfType<NOMAD::MegaIteration*>()->setSuccessType(_templateAlgoRandom->getSuccessType());
    }

    // End of the iteration: iterationSuccess is true if we have a success.
    return iterationSuccess;

}
