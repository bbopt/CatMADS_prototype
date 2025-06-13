
#include <algorithm>

#include "MySimpleMads.hpp"
#include "Algos/SubproblemManager.hpp"
#include "Cache/CacheBase.hpp"


void MySimpleMads::init()
{
    setStepType(NOMAD::StepType::ALGORITHM_MADS);
    
    // We can accept Mads with more than one objective when doing a PhaseOneSearch of DMultiMads optimization.
    if (!_runParams->getAttributeValue<bool>("DMULTIMADS_OPTIMIZATION") && NOMAD::Algorithm::getNbObj() > 1)
    {
        throw NOMAD::InvalidParameter(__FILE__,__LINE__,"My Simple Mads solves single objective problems. To handle several objectives please use DMultiMads: DMULTIMADS_OPTIMIZATION yes");
    }

    if (nullptr == _poll.getBarrier())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"My simple mads must have a poll barrier.");
    }
    
    _minFrameSize = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("MIN_FRAME_SIZE");
    

}

bool MySimpleMads::runImp()
{
    size_t k = 0;   // Iteration number (incremented at start)
    
    bool pollSuccess = false;
    _myPollSuccessType = MySuccessType::FULL_SUCCESS;
    //bool frameSizeLowerThanMin = false;
    
    
    // Termination: 1- Reach max model eval,  2- my full success of poll
    //while ( _poll.getNbEval() < _maxEval && !frameSizeLowerThanMin && _myPollSuccessType == MySuccessType::FULL_SUCCESS)
    while ( _poll.getNbEval() < _maxEval && _myPollSuccessType == MySuccessType::FULL_SUCCESS)
    {
        _poll.start();
        pollSuccess = _poll.run();
        _poll.end();
        k++;
        
        // Upcast the type of success of MySimplePoll to MySimpleMads to treat it with the Extended poll
        _myPollSuccessType = _poll.getMySuccessType();
        
        // Mesh size stopping criterion
        //auto currentFrameSizeTMP = _poll.getMesh()->getDeltaFrameSize();
        //frameSizeLowerThanMin = _poll.getMesh()->getDeltaFrameSize() <= _minFrameSize;
    }

    return pollSuccess;
}

const NOMAD::SimpleEvalPoint & MySimpleMads::getBestSimpleSolution(bool bestFeas) const
{
    
    const auto & barrier = _poll.getBarrier();
    // bool runInPhaseOneSeach = _poll.getPhaseOneSearch();

    if (bestFeas) //  && !runInPhaseOneSeach)
    {
        return barrier->getCurrentIncumbentFeas();
    }
    else
    {
        return barrier->getCurrentIncumbentInf();

    }
    
}


NOMAD::EvalPoint MySimpleMads::getBestSolution(bool bestFeas) const
{
    NOMAD::SimpleEvalPoint bestSimple = getBestSimpleSolution(bestFeas);
    NOMAD::EvalPoint bestSol(bestSimple);
    
    // Make best sol in full space to be able to find it in Cache
    auto fixedVariable = NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this);
    bestSol = bestSol.makeFullSpacePointFromFixed(fixedVariable);
    
    // Get eval from cache
    NOMAD::CacheBase::getInstance()->find(*(bestSol.getX()), bestSol);
    
    return bestSol;
}


void MySimpleMads::endImp()
{
    
    if ( _endDisplay )
    {
        endDisplay();
    }
}


void MySimpleMads::endDisplay() const
{
    // Display best feasible solutions.
    std::string sFeas;
    
    // Output level is info if this algorithm is a sub part of another algorithm.
    NOMAD::OutputLevel outputLevel = NOMAD::OutputLevel::LEVEL_INFO;
    
    if ( ! NOMAD::OutputQueue::GoodLevel(outputLevel) )
    {
        return;
    }
    
    NOMAD::FHComputeTypeS computeType; /*default initializer is used*/
    
    auto solFormat = NOMAD::OutputQueue::getInstance()->getSolFormat();
    
    NOMAD::OutputInfo displaySolFeas(getName(), sFeas, outputLevel);
    
    sFeas = "Best feasible solution";
    
    auto bestFeas = getBestSolution(true);
    
    if (!bestFeas.isComplete())
    {
        sFeas += ":     Undefined.";
        displaySolFeas.addMsg(sFeas);
    }
    else
    {
        sFeas += ":     ";
        displaySolFeas.addMsg(sFeas + bestFeas.display(computeType,
                                                       solFormat,
                                                       NOMAD::DISPLAY_PRECISION_FULL,
                                                       false));
    }
    
    NOMAD::OutputQueue::Add(std::move(displaySolFeas));
    
    // Display best infeasible solutions.
    std::string sInf;
    NOMAD::OutputInfo displaySolInf(getName(), sInf, outputLevel);
    sInf = "Best infeasible solution";
    
    auto bestInf = getBestSolution(false);
    
    if ( !bestInf.isComplete())
    {
        sInf += ":   Undefined.";
        displaySolInf.addMsg(sInf);
    }
    else
    {
        sInf += ":   ";
        displaySolInf.addMsg(sInf + bestInf.display(computeType,
                                                    solFormat,
                                                    NOMAD::DISPLAY_PRECISION_FULL,
                                                    false));
    }
    NOMAD::OutputQueue::Add(std::move(displaySolInf));
        
    std::string sNbEval = "Function evaluations: " + NOMAD::itos(_poll.getNbEval());
    NOMAD::OutputQueue::Add(std::move(sNbEval));
}

