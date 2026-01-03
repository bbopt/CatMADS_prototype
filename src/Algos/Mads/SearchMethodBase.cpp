
#include "../../Algos/Mads/SearchMethodBase.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::SearchMethodBase::init()
{
    // A search method must have a parent
    verifyParentNotNull();
    
    if (nullptr != _pbParams)
    {
        _lb = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND");
        _ub = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND");
    }

}


void NOMAD::SearchMethodBase::endImp()
{
    // Replace default SuccessType by actual success type
    // When not dynamicEnabled the success type is UNDEFINED.
    if (_dynamicSearch)
    {
        _allSuccessTypes.back() =_trialPointsSuccess;
    }
    
    // Compute hMax and update incumbents in barrier only if full success.
    // If no trial points success, just add the points in barrier.
    // HMax and incumbents update will be performed during poll.
    
    _updateIncumbentsAndHMax = (_trialPointsSuccess >= NOMAD::SuccessType::FULL_SUCCESS);
    postProcessing();

    // Need to reimplement end() to set a stop reason for Mads based on the search method stop reason
}


void NOMAD::SearchMethodBase::generateTrialPointsImp()
{

    OUTPUT_INFO_START
    AddOutputInfo("Generate points for " + getName(), true, false);
    OUTPUT_INFO_END
    
    generateTrialPointsFinal();

    // Snap the points to bounds and mesh
    const auto& searchMethodPoints = getTrialPoints();
    
    std::list<NOMAD::EvalPoint> snappedTrialPoints;
    for (auto evalPoint : searchMethodPoints)
    {
        if (snapPointToBoundsAndProjectOnMesh(evalPoint, _lb, _ub))
        {
            snappedTrialPoints.push_back(evalPoint);
            OUTPUT_INFO_START
            std::string s = "Snap point " + evalPoint.display();
            AddOutputInfo(s);
            OUTPUT_INFO_END
        }
    }

    // Re-insert snapped trial points
    clearTrialPoints();
    for (const auto& evalPoint : snappedTrialPoints)
    {
        insertTrialPoint(evalPoint);
    }
    
    OUTPUT_INFO_START
    AddOutputInfo("Generated " + std::to_string(getTrialPointsCount()) + " points");
    AddOutputInfo("Generate points for " + getName(), false, true);
    OUTPUT_INFO_END

}
