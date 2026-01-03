/**
\file   SuccessStats.cpp
\brief  Manage step success stats.
\author Christophe Tribes
\date   June 2022
*/

#include "../Eval/SuccessStats.hpp"

// This function is used for incrementing the SuccessStats of a step
void NOMAD::SuccessStats::updateStats(SuccessType successType, StepType stepType ,size_t val )
{

    // UNDEFINED is not used for stats
    if ( NOMAD::SuccessType::UNDEFINED == successType )
    {
        return;
    }

    setNbConsecutiveSuccessAndFail(successType,val);

    updateSuccessAndFailStats(successType, stepType, val);
}

void NOMAD::SuccessStats::updateSuccessAndFailStats(SuccessType successType, StepType stepType ,size_t val )
{

    auto p = std::make_pair(stepType,successType);
    auto iter = _nbSuccessAndFail.find(p);

    // First time insert
    if (iter == _nbSuccessAndFail.end())
    {
        _nbSuccessAndFail.insert(std::make_pair(p, val));
    }
    else
    {
        _nbSuccessAndFail.at(p) += val;
    }
}


// This function is used for propagation.
void NOMAD::SuccessStats::updateStats(const SuccessStats & evalStats )
{
    // We may have more stats
    const auto & statsMap = evalStats.getStatsMapSuccessAndFail();
    for (const auto& it : statsMap)
    {
        auto p = it.first;
        auto val = it.second;
        updateSuccessAndFailStats(p.second, p.first,val);
    }

}

void NOMAD::SuccessStats::setNbConsecutiveSuccessAndFail(SuccessType successType, size_t val)
{


    if (successType >= NOMAD::SuccessType::PARTIAL_SUCCESS)
    {
        // Increment success count for this step and reset fail count
        _nbConsecutiveSuccess +=val;
        _nbConsecutiveFail = 0;
    }
    else
    {
        // Increment fail count for this step and reset success count
        _nbConsecutiveFail += val;
        _nbConsecutiveSuccess = 0;
    }
}

size_t NOMAD::SuccessStats::getStat(StepType stepType, SuccessType successType) const
{
    auto iter = _nbSuccessAndFail.find(std::make_pair(stepType,successType));

    if (iter == _nbSuccessAndFail.end())
    {
        return 0;
    }
    else
    {
        return iter->second;
    }
}


/*---------------*/
/*    display    */
/*---------------*/
std::string NOMAD::SuccessStats::display() const
{
    std::string s;
    throw NOMAD::Exception(__FILE__, __LINE__,"Not yet implemented ");
    return s;
}

std::ostream& operator<<(std::ostream& os, NOMAD::SuccessStats& stats)
{

    std::ostringstream oss;
    oss << stats.display();
    return os;
}
