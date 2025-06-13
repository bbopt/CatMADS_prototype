
#include "../../Algos/Mads/SearchMethodSimple.hpp"

bool NOMAD::SearchMethodSimple::runImp()
{

    bool foundBetter = false;
    if ( ! _stopReasons->checkTerminate() && !_trialPoints.empty())
    {
        foundBetter = evalTrialPoints(this);
    }
    else
    {
        // Stop reason is triggered. Step is unsuccessful
        _success = NOMAD::SuccessType::UNSUCCESSFUL;
    }

    return foundBetter;
}


void NOMAD::SearchMethodSimple::startImp()
{
    // Store default successType for stats
    // Only if not _dynamicEnabled, the success type will stay unchanged.
    if (_dynamicSearch)
    {
        _allSuccessTypes.push_back(NOMAD::SuccessType::UNDEFINED);
    }
    _trialPointsSuccess = NOMAD::SuccessType::UNDEFINED;
    
    
    // Reset the current counters. The total counters are not reset (done only once when constructor is called).
    _trialPointStats.resetCurrentStats();
    
    if ( ! _stopReasons->checkTerminate() && _dynamicSearchEnabled)
    {
        // Create EvalPoints and snap to bounds and snap on mesh
        generateTrialPoints();
        
        // Complete trial points information to get ready for sorting and evaluation
        completeTrialPointsInformation();
    }
}

void NOMAD::SearchMethodSimple::endImp()
{
    NOMAD::SearchMethodBase::endImp();

    // For now, only simple search method can be set dynamically enabled/disabled.
    if (_dynamicSearch)
    {
        updateDynamicEnabled();
    }
    
    // When users apply some changes to the problem (for example, fixed variables), it may be required to revert the change.
    // Some changes can be made after trial points evaluations
    updateAtStepEnd();
}
