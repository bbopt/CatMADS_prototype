
#include "../Util/AllStopReasons.hpp"

NOMAD::StopReason<NOMAD::BaseStopType> NOMAD::AllStopReasons::_baseStopReason = NOMAD::StopReason<NOMAD::BaseStopType>();
NOMAD::StopReason<NOMAD::EvalGlobalStopType> NOMAD::AllStopReasons::_evalGlobalStopReason = NOMAD::StopReason<NOMAD::EvalGlobalStopType>();

void NOMAD::AllStopReasons::setStarted()
{
    _baseStopReason.setStarted();
    _evalGlobalStopReason.setStarted();
    _iterStopReason.setStarted();
}


std::string NOMAD::AllStopReasons::getStopReasonAsString() const
{
    std::string stopReason;
    bool flagTerminate = false;

    if (_baseStopReason.checkTerminate())
    {
        stopReason += getBaseStopReasonAsString();
        flagTerminate = true;
    }

    if (_evalGlobalStopReason.checkTerminate())
    {
        stopReason += (stopReason.empty() ? "" : " ") + getEvalGlobalStopReasonAsString();
        flagTerminate = true;
    }

    if (_iterStopReason.checkTerminate())
    {
        stopReason += (stopReason.empty() ? "" : " ") + _iterStopReason.getStopReasonAsString() + " (IterStopType)";
        flagTerminate = true;
    }


    if (!flagTerminate)
    {
        stopReason = "No termination (all). ";
    }

    return stopReason;

}


std::string NOMAD::AllStopReasons::getEvalGlobalStopReasonAsString()
{
    return _evalGlobalStopReason.getStopReasonAsString() + " (Eval Global)";
}


std::string NOMAD::AllStopReasons::getBaseStopReasonAsString()
{
    return _baseStopReason.getStopReasonAsString() + " (Base)";
}


bool NOMAD::AllStopReasons::checkTerminate() const
{
    return ( _baseStopReason.checkTerminate()
            || _evalGlobalStopReason.checkTerminate()
            || _iterStopReason.checkTerminate());
}


