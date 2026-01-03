
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Util/fileutils.hpp"

#include "../../Algos/Mads/SimpleLineSearchMethod.hpp"
#include "../../Algos/SimpleLineSearch/SimpleLineSearch.hpp"
#include "../../Algos/SimpleLineSearch/SimpleLineSearchMegaIteration.hpp"

void NOMAD::SimpleLineSearch::init()
{

    setStepType(NOMAD::StepType::SEARCH_METHOD_SIMPLE_LINE_SEARCH);
    verifyParentNotNull();

    const auto parentSearch = getParentOfType<NOMAD::SimpleLineSearchMethod*>(false);

    if (nullptr == parentSearch)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearch: cannot be a standalone method. Must be part of a mads search.");
    }
    
    
}

bool NOMAD::SimpleLineSearch::runImp()
{
    _algoSuccessful = false;
    
    
    if ( ! _stopReasons->checkTerminate() )
    {
        auto barrier = getParentOfType<NOMAD::MegaIteration*>()->getBarrier();
        
        if (nullptr == barrier)
        {
            throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearch needs a barrier from a Mega Iteration");
        }
        
        // Create a single MegaIteration: manage multiple iterations.
        NOMAD::SimpleLineSearchMegaIteration megaIteration(this, 0, barrier,NOMAD::SuccessType::UNDEFINED);
        
        megaIteration.start();
        bool currentMegaIterSuccess = megaIteration.run();
        megaIteration.end();
        
        _algoSuccessful = _algoSuccessful || currentMegaIterSuccess;
        
        auto algoStopReason = NOMAD::AlgoStopReasons<NOMAD::SimpleLineSearchStopType>::get ( _stopReasons );
        algoStopReason->set( NOMAD::SimpleLineSearchStopType::ALL_POINTS_EVALUATED); // This will stop iterations.
    
        // _refMegaIteration is used to keep values used in Mads::end(). Update it here.
        _refMegaIteration = std::make_shared<NOMAD::SimpleLineSearchMegaIteration>(this, 0, barrier, _success);
        
        _termination->start();
        _termination->run();
        _termination->end();
    }
    
    return _algoSuccessful;
}


void NOMAD::SimpleLineSearch::readInformationForHotRestart()
{
    // TODO...maybe
    if (_runParams->getAttributeValue<bool>("HOT_RESTART_READ_FILES"))
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"SimpleLineSearch: cannot be used with hot restart.");
    }
}

