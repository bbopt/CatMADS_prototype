
#include "../../Algos/QuadModelSLD/QuadModelSldAlgo.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldIteration.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldOptimize.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldUpdate.hpp"


void NOMAD::QuadModelSldIteration::init()
{

    // Count the number of constraints
    const auto bbot = NOMAD::Algorithm::getBbOutputType();

    // Init the TrainingSet
    size_t n = _pbParams->getAttributeValue<size_t>("DIMENSION");

    // The quadratic model
    _model = std::shared_ptr<NOMAD::QuadModelSld>(new NOMAD::QuadModelSld(bbot ,n));
    
    if (_trialPoints.size() > 0)
    {
        _useForSortingTrialPoints = true;
        setStepType(NOMAD::StepType::QUAD_MODEL_SORT);
    }

}

std::string NOMAD::QuadModelSldIteration::getName() const
{
    if (_useForSortingTrialPoints)
    {
        return NOMAD::stepTypeToString(_stepType) + " #" + std::to_string(_k);
    }
    else
    {
        return NOMAD::Iteration::getName();
    }
}


void NOMAD::QuadModelSldIteration::startImp()
{

    // Select the sample points to construct the model. Use a center pt and the cache
    
    NOMAD::QuadModelSldUpdate update(this, _trialPoints);
    update.start();
    bool updateSuccess = update.run();
    update.end();

    if ( ! updateSuccess && ! _useForSortingTrialPoints)
    {
        auto qmsStopReason = NOMAD::AlgoStopReasons<NOMAD::ModelStopType>::get ( getAllStopReasons() );

        // The initial update is not a success. If the stop reason is not set to terminate we set a default stop reason for initialization.
        if ( !_stopReasons->checkTerminate() )
            qmsStopReason->set( NOMAD::ModelStopType::INITIAL_FAIL);
    }
}


bool NOMAD::QuadModelSldIteration::runImp()
{

    bool iterationSuccess = false;

    // Initialize optimize member on model
    NOMAD::QuadModelSldOptimize optimize (this, _pbParams);

    // Model Update is handled in start().
    if (!_stopReasons->checkTerminate() && _model->check() )
    {
        // Optimize to find oracle points on this model
        optimize.start();
        iterationSuccess = optimize.run();
        optimize.end();
    }

    // Update MegaIteration success type
    NOMAD::SuccessType success = optimize.getSuccessType();
    auto megaIter = getParentOfType<NOMAD::MegaIteration*>();
    megaIter->setSuccessType(success);

    // End of the iteration: iterationSuccess is true if we have a success.
    return iterationSuccess;

}
