
#include "../../Algos/Mads/SimpleLineSearchMethod.hpp"
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/SimpleLineSearch/SimpleLineSearch.hpp"


void NOMAD::SimpleLineSearchMethod::init()
{
    // For some testing, it is possible that _runParams is null or evaluator control is null

    if ( nullptr != _runParams && nullptr != NOMAD::EvcInterface::getEvaluatorControl() )
    {
        setStepType(NOMAD::StepType::SEARCH_METHOD_SIMPLE_LINE_SEARCH);

        bool enabled = _runParams->getAttributeValue<bool>("SIMPLE_LINE_SEARCH");
        
        if (enabled && _runParams->getAttributeValue<bool>("SPECULATIVE_SEARCH"))
        {
            throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearchMethod: cannot work with speculative search.");
        }
        
        setEnabled(enabled);

        
        _simpleLineSearchStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::SimpleLineSearchStopType>>();
        
        _simpleLineSearch = std::make_unique<NOMAD::SimpleLineSearch>(this,
                                            _simpleLineSearchStopReasons ,
                                            _runParams,
                                            _pbParams);
        
    }
}


bool NOMAD::SimpleLineSearchMethod::runImp()
{
   
    _simpleLineSearch->setEndDisplay(false);

    _simpleLineSearch->start();
    bool foundBetter = _simpleLineSearch->run();
    _simpleLineSearch->end();

    // Maybe use _simpleLineSearchStopReason to update parent algorithm
    
    return foundBetter;
}


void NOMAD::SimpleLineSearchMethod::generateTrialPointsFinal()
{
    throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearchMethod: cannot work with MegaSearchPoll.");
}
