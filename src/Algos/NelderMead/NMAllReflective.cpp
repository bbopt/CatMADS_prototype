
#include "../../Algos/NelderMead/NMAllReflective.hpp"
#include "../../Algos/NelderMead/NMReflective.hpp"
#include "../../Algos/SubproblemManager.hpp"


std::string NOMAD::NMAllReflective::getName() const
{
    return "Single NM Iteration";
}

void NOMAD::NMAllReflective::startImp()
{
    if ( ! _stopReasons->checkTerminate() )
    {
        // The iteration start function manages the simplex creation.
        NMIteration::startImp();

        // Generate REFLECT, EXPANSION, INSIDE_CONTRACTION, OUTSIDE_CONTRACTION (no SHRINK)
        // All points are generated before evaluation
        verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, true);

        generateTrialPoints();
        if (_projectOnMesh && !verifyPointsAreOnMesh(getName()))
        {
            OUTPUT_INFO_START
            AddOutputInfo("At least one trial point is not on mesh. May need investigation if this happens too often.");
            OUTPUT_INFO_END
        }
    }
}


void NOMAD::NMAllReflective::generateTrialPointsImp ()
{
    
    // Single constructor call move into init (may need refactor due to NMIterationUtils)
    NOMAD::NMReflective reflect( this );

    // Need to set the current step type before starting
    reflect.setCurrentNMStepType( NOMAD::StepType::NM_REFLECT );

    // Create trial points but no evaluation
    reflect.start();
    reflect.end();
    {
        const auto& trialPts = reflect.getTrialPoints();
        for (auto evalPoint : trialPts)
        {
            evalPoint.addGenStep(getStepType());
            insertTrialPoint(evalPoint);
        }
    }

    // Expand simplex
    if ( ! _stopReasons->checkTerminate() )
    {
        reflect.setCurrentNMStepType( NOMAD::StepType::NM_EXPAND );
        reflect.start();
        reflect.end();
        const auto& trialPts = reflect.getTrialPoints();
        for (auto evalPoint : trialPts)
        {
            evalPoint.addGenStep(getStepType());
            insertTrialPoint(evalPoint);
        }

    }

    // Inside contraction of simplex
    if ( ! _stopReasons->checkTerminate() )
    {
        reflect.setCurrentNMStepType( NOMAD::StepType::NM_INSIDE_CONTRACTION );
        reflect.start();
        reflect.end();
        const auto& trialPts = reflect.getTrialPoints();
        for (auto evalPoint : trialPts)
        {
            evalPoint.addGenStep(getStepType());
            insertTrialPoint(evalPoint);
        }

    }

    // Outside contraction of simplex
    if ( ! _stopReasons->checkTerminate() )
    {
        reflect.setCurrentNMStepType( NOMAD::StepType::NM_OUTSIDE_CONTRACTION );
        reflect.start();
        reflect.end();
        const auto& trialPts = reflect.getTrialPoints();
        for (auto evalPoint : trialPts)
        {
            evalPoint.addGenStep(getStepType());
            insertTrialPoint(evalPoint);
        }

    }

    // If everything is ok we terminate a single NM iteration completed anyway
    if ( ! _stopReasons->checkTerminate() )
    {
        auto nmStopReason = NOMAD::AlgoStopReasons<NOMAD::NMStopType>::get ( getAllStopReasons() );
        nmStopReason->set(NOMAD::NMStopType::NM_SINGLE_COMPLETED);
    }
}
