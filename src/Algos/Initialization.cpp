#include "../Algos/Initialization.hpp"
#include "../Cache/CacheBase.hpp"
#include "../Output/OutputQueue.hpp"

NOMAD::Initialization::~Initialization()
{
    NOMAD::OutputQueue::Flush();
}


void NOMAD::Initialization::init()
{
    setStepType(NOMAD::StepType::INITIALIZATION);
    verifyParentNotNull();
    
    _x0s = _pbParams->getAttributeValue<NOMAD::ArrayOfPoint>("X0");
    _n = _pbParams->getAttributeValue<size_t>("DIMENSION");
    
}


std::string NOMAD::Initialization::getName() const
{
    return getAlgoName() + NOMAD::stepTypeToString(_stepType);
}

void NOMAD::Initialization::endImp()
{
    _trialPointStats.updateParentStats();
}

void NOMAD::Initialization::incrementCounters()
{
    // Increment number of calls to start, run and end sequence.
    _trialPointStats.incrementNbCalls();
}

void NOMAD::Initialization::validateX0s() const
{

    bool validX0available = false;
    std::string err;

    for (const auto& x0 : _x0s)
    {
        if (!x0.isComplete() || x0.size() != _n)
        {
            err += "Initialization: eval_x0s: Invalid X0 " + x0.display() + ".";
        }
        else
        {
            validX0available = true;
        }
    }

    if (validX0available)
    {
        if (!err.empty())
        {
            // Show invalid X0s
            AddOutputWarning(err);
        }
    }
    else
    {
        // No valid X0 available. Throw exception.
        size_t cacheSize = NOMAD::CacheBase::getInstance()->size();
        if (cacheSize > 0)
        {
            err += " Hint: Try not setting X0 so that the cache is used (";
            err += std::to_string(cacheSize) + " points).";
        }
        else
        {
            err += " Cache is empty. Hint: Try setting LH_SEARCH so that the Latin Hypercube Search is used to find initial points.";
        }
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }

}
