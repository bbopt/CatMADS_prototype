
#include "../../Algos/Mads/TemplateAlgoSearchMethod.hpp"
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgo.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoSinglePass.hpp"

void NOMAD::TemplateAlgoSearchMethod::init()
{
    // For some testing, it is possible that _runParams is null or evaluator control is null
    bool randomAlgoSearch = false;
    if ( nullptr != _runParams && nullptr != NOMAD::EvcInterface::getEvaluatorControl() )
    {
        if ( _runParams->getAttributeValue<bool>("MEGA_SEARCH_POLL") )
        {
            setStepType(NOMAD::StepType::SEARCH_METHOD_ALGO_RANDOM);
        }
        else
        {
            setStepType(NOMAD::StepType::ALGORITHM_RANDOM);
        }
        // TEMPLATE use for a new search method: a new parameter must be defined to enable or not the search method (see ../Attributes/runAttributesDefinition.txt)
        randomAlgoSearch = _runParams->getAttributeValue<bool>("RANDOM_ALGO_SEARCH");
    }
    setEnabled(randomAlgoSearch);
    
    
    if (randomAlgoSearch)
    {
        // TEMPLATE for a new search method: parameters can be defined to control the search method (see ../Attributes/runAttributesDefinition.txt)
        auto dummyFactor = _runParams->getAttributeValue<size_t>("RANDOM_ALGO_DUMMY_FACTOR");
        auto dim = _pbParams->getAttributeValue<size_t>("DIMENSION");
        if (dummyFactor < NOMAD::INF_SIZE_T)
        {
            NOMAD::EvcInterface::getEvaluatorControl()->setLapMaxBbEval( dim*dummyFactor ); // In this example, the single pass (lap) max bb eval is set.
        }
        
        // The algorithm has its own stop reasons.
        // TEMPLATE for a new search method: adapt for the new Algo.
        _randomAlgoStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::RandomAlgoStopType>>();
        
        // Create the algorithm with its own stop reason
        // TEMPLATE use for a new search method: adapt for the new Algo.
        _randomAlgo = std::make_unique<NOMAD::TemplateAlgo>(this,
                                              _randomAlgoStopReasons ,
                                              _runParams,
                                              _pbParams);
        
    }
}


bool NOMAD::TemplateAlgoSearchMethod::runImp()
{
   
    _randomAlgo->setEndDisplay(false);

    _randomAlgo->start();
    bool foundBetter = _randomAlgo->run();
    _randomAlgo->end();

    // Maybe use _randomAlgoStopReason to update parent algorithm
    
    return foundBetter;
}


void NOMAD::TemplateAlgoSearchMethod::generateTrialPointsFinal()
{
    // The trial points of one iteration of this template algorithm (random) are generated (not evaluated).


    // Note: Use first point of barrier as simplex center.
    // See issue #392
    NOMAD::TemplateAlgoSinglePass randomAlgo(this,
                                             getMegaIterationBarrier()->getFirstPoint());
    randomAlgo.start();
    randomAlgo.end();

    // Pass the generated trial pts to this
    const auto& trialPtsNM = randomAlgo.getTrialPoints();
    for (const auto& point : trialPtsNM)
    {
        insertTrialPoint(point);
    }

}
