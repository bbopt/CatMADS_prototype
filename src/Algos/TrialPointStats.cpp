/**
\file   TrialPointStats.cpp
\brief  Manage stats for point generation methods (ex. Search and Poll)
\author Christophe Tribes
\date   August 2021
*/

#include "../Algos/Algorithm.hpp"
#include "../Algos/IterationUtils.hpp"
#include "../Algos/TrialPointStats.hpp"

void NOMAD::TrialPointStats::init()
{    
    _nbCalls = 0;
    
    initializeMap(_nbTotalEvalsDone);
    initializeMap(_nbCurrentEvalsDone);
    
    initializeMap(_nbTotalTrialPointsGenerated);
    initializeMap(_nbCurrentTrialPointsGenerated);
    
}

void NOMAD::TrialPointStats::initializeMap(std::map<EvalType, size_t> & counter)
{
    counter.clear();
    for (auto et: _allEvalType )
    {
        counter.insert(std::make_pair(et, 0));
    }
        
}

void NOMAD::TrialPointStats::incrementEvalsDone(size_t nb, EvalType evalType)
{
    if ( evalType < NOMAD::EvalType::LAST)
    {
        _nbTotalEvalsDone.at(evalType) += nb;
        _nbCurrentEvalsDone.at(evalType) += nb;
    }
}

void NOMAD::TrialPointStats::incrementTrialPointsGenerated(size_t nb, EvalType evalType)
{
    if ( evalType < NOMAD::EvalType::LAST)
    {
        _nbTotalTrialPointsGenerated.at(evalType) += nb;
        _nbCurrentTrialPointsGenerated.at(evalType) += nb;
    }
}

size_t NOMAD::TrialPointStats::getNbTrialPointsGenerated(EvalType evalType, bool totalCount) const
{
    if (totalCount)
        return size_t(_nbTotalTrialPointsGenerated.at(evalType));
    else
        return size_t(_nbCurrentTrialPointsGenerated.at(evalType));
}

size_t NOMAD::TrialPointStats::getNbEvalsDone(EvalType evalType, bool totalCount) const
{
    if (totalCount)
        return size_t(_nbTotalEvalsDone.at(evalType));
    else
        return size_t(_nbCurrentEvalsDone.at(evalType));
}

void NOMAD::TrialPointStats::updateWithCurrentStats(const TrialPointStats &trialPointStats)
{
    // Use the CURRENT stats of the given trialPointStats to update this current trialPointStats (CURRENT and TOTAL)
    for (auto et: _allEvalType )
    {
        _nbTotalEvalsDone.at(et) += trialPointStats.getNbEvalsDone(et, false);
        _nbCurrentEvalsDone.at(et) += trialPointStats.getNbEvalsDone(et, false);
        
        _nbTotalTrialPointsGenerated.at(et) += trialPointStats.getNbTrialPointsGenerated(et, false);
        _nbCurrentTrialPointsGenerated.at(et) += trialPointStats.getNbTrialPointsGenerated(et, false);
    }
}

void NOMAD::TrialPointStats::resetCurrentStats()
{
    for (auto et: _allEvalType )
    {
        _nbCurrentEvalsDone[et] = 0 ;
        _nbCurrentTrialPointsGenerated[et] = 0;
    }
    
}

void NOMAD::TrialPointStats::updateParentStats()
{
    // First try to update iteration utils parent
    // The parent can be an IterationUtils using an Algorithm to generate and evaluate trial point.
    // For example, VNS Search Method (IU) runs a VNS (Algo) which runs a Mads (Algo), etc. We need to pass the stats from Mads to VNS and from VNS to VNS Search Method.

    Step* step = const_cast<Step*>(_parentStep);
    while (nullptr != step)
    {
        if (nullptr != dynamic_cast<NOMAD::IterationUtils*>(step))
        {
            auto iu = dynamic_cast<NOMAD::IterationUtils*>(step);
            iu->updateStats(*this); // ChT. Don't think a critical region is needed here
            break;
        }
        else if (nullptr != dynamic_cast<NOMAD::Algorithm*>(step))
        {
            auto algo = dynamic_cast<NOMAD::Algorithm*>(step);
#ifdef _OPENMP
            // Critical region is needed for parallel algo like psdmads. PSDMads algo gets update from multiple Mads
#pragma omp critical(updateLock)
#endif // _OPENMP
            {
                algo->updateStats(*this);
            }
            break;
        }
        step = const_cast<Step*>(step->getParentStep());
    }
}

/*---------------*/
/*    display    */
/*---------------*/
std::string NOMAD::TrialPointStats::display() const
{
    std::string s;
    throw NOMAD::Exception(__FILE__, __LINE__,"Not yet implemented ");
    return s;
}

std::ostream& operator<<(std::ostream& os, NOMAD::TrialPointStats& stats)
{
    
    std::ostringstream oss;
    oss << stats.display();
    return os;
}
