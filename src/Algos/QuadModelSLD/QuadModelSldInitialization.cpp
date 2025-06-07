
#include "../../Cache/CacheBase.hpp"
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/QuadSearchMethod.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldInitialization.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::QuadModelSldInitialization::init()
{
    _qmStopReason = NOMAD::AlgoStopReasons<NOMAD::ModelStopType>::get( _stopReasons );

}


/*-------------------------*/
/*       Destructor        */
/*-------------------------*/
NOMAD::QuadModelSldInitialization::~QuadModelSldInitialization()
{
}


void NOMAD::QuadModelSldInitialization::startImp()
{

    if ( ! _stopReasons->checkTerminate() )
    {
        // For a standalone optimization (no Search Method), X0s points must be evaluated if available, otherwise, the cache can be used.
        // Do nothing if this is part of a Search Method.

        auto searchMethodConst = getParentOfType<NOMAD::QuadSearchMethod*>(false);

        if ( searchMethodConst == nullptr )
        {
            // The name generateTrialPoints is not well suited here because we use provided X0s and check provided cache.
            generateTrialPoints();
        }
    }

}


bool NOMAD::QuadModelSldInitialization::runImp()
{
    bool doContinue = ! _stopReasons->checkTerminate();

    // For a standalone optimization (no Search method), X0s points must be evaluated if available, otherwise, the cache can be used.
    // Do nothing if this is part of a sub-optimization.
    auto searchMethodConst = getParentOfType<NOMAD::QuadSearchMethod*>(false);

    if ( doContinue && searchMethodConst == nullptr )
    {
        // For a standalone quad model optimization, evaluate the X0s
        bool evalOk = eval_x0s();

        doContinue = ! _stopReasons->checkTerminate();
        if ( ! doContinue || ! evalOk )
            _qmStopReason->set(NOMAD::ModelStopType::X0_FAIL);

    }
    return doContinue;
}


// The name generateTrialPoints is not well suited here because we use provided X0s and check provided cache.
void NOMAD::QuadModelSldInitialization::generateTrialPointsImp()
{
    auto x0s = _pbParams->getAttributeValue<NOMAD::ArrayOfPoint>("X0");
    size_t n = _pbParams->getAttributeValue<size_t>("DIMENSION");
    bool validX0available = false;
    std::string err;

    for (auto x0 : x0s )
    {
        if (!x0.isComplete() || x0.size() != n)
        {
            err += "Initialization: eval_x0s: Invalid X0 " + x0.display() + ".";
        }
        else
        {
            // New EvalPoint to be evaluated.
            // Add it to the list (local or in Search method).
            validX0available = insertTrialPoint(NOMAD::EvalPoint(x0));;
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
        // No valid X0 available, no cache. Throw exception.
        size_t cacheSize = NOMAD::CacheBase::getInstance()->size();
        if (cacheSize > 0)
        {
            err += " Hint: Try not setting X0 so that the cache is used (";
            err += std::to_string(cacheSize) + " points)";
        }
        else
        {
            err += ". Cache is empty.";
        }
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }

}


// Eval X0s, using blackbox.
// Method is copied from MadsInitialization.
bool NOMAD::QuadModelSldInitialization::eval_x0s()
{
    bool evalOk = false;

    // Add X0s that need evaluation to eval queue
    NOMAD::EvcInterface evcInterface(this);
    auto evc = evcInterface.getEvaluatorControl();

    // Enforce no opportunism.
    auto previousOpportunism = evc->getOpportunisticEval();
    evc->setOpportunisticEval(false);

    // Evaluate all x0s. Ignore returned success type.
    // Note: EvaluatorControl would not be able to compare/compute success since there is no barrier.
    evalOk = evalTrialPoints(this);

    // Reset opportunism to previous values.
    evc->setOpportunisticEval(previousOpportunism);

    NOMAD::OutputQueue::Flush();

    return evalOk;
}
