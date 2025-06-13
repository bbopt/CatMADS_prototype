
#include "../../Algos/QuadModel/QuadModelAlgo.hpp"
#include "../../Algos/QuadModel/QuadModelIteration.hpp"
#include "../../Algos/QuadModel/QuadModelOptimize.hpp"
#include "../../Algos/QuadModel/QuadModelUpdate.hpp"
#include "../../../ext/sgtelib/src/Surrogate_Factory.hpp"


void NOMAD::QuadModelIteration::init()
{

    // Count the number of constraints
    const auto bbot = NOMAD::Algorithm::getBbOutputType();
    
    size_t nbModels = bbot.size(); // One output, one quad "model"
    if ( _flagPriorCombineObjsForModel )
    {
        // Nb models = nb constraint + 1 objective
        nbModels = NOMAD::getNbConstraints(bbot) + 1;
    }

    // Init the TrainingSet
    size_t n = _pbParams->getAttributeValue<size_t>("DIMENSION");
    SGTELIB::Matrix empty_X("empty_X", 0, static_cast<int>(n));
    SGTELIB::Matrix empty_Z("empty_Z", 0, static_cast<int>(nbModels));
    _trainingSet = std::make_shared<SGTELIB::TrainingSet>(empty_X, empty_Z);

    // The quadratic model is from Sgtelib
    _model = std::shared_ptr<SGTELIB::Surrogate>(SGTELIB::Surrogate_Factory(*_trainingSet, "TYPE PRS RIDGE 0"));
    
    if (!_trialPoints.empty())
    {
        _useForSortingTrialPoints = true;
        setStepType(NOMAD::StepType::QUAD_MODEL_SORT);
    }
    

}

std::string NOMAD::QuadModelIteration::getName() const
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


void NOMAD::QuadModelIteration::startImp()
{

    // Select the sample points to construct the model. Use a center pt and the cache
    
    NOMAD::QuadModelUpdate update(this, std::vector<Direction>() /* no scaling directions */, _trialPoints);
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


bool NOMAD::QuadModelIteration::runImp()
{

    bool iterationSuccess = false;

    // Initialize optimize member on model
    NOMAD::QuadModelOptimize optimize (this, _pbParams, false /* do not perform on a scaled models */);

    // Model Update is handled in start().
    if (!_stopReasons->checkTerminate() && _model->is_ready() )
    {
        // Optimize to find oracle points on this model
        optimize.start();
        iterationSuccess = optimize.run();
        optimize.end();
    }

    // TODO check that the success type of optimize is passed to mega iteration
    // Update MegaIteration success type
    _success = optimize.getTrialPointsSuccessType();
    auto megaIter = getParentOfType<NOMAD::MegaIteration*>();
    megaIter->setSuccessType(_success);

    // End of the iteration: iterationSuccess is true if we have a success.
    return iterationSuccess;

}
