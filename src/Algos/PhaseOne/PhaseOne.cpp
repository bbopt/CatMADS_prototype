
#include "../../Algos/CacheInterface.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/PhaseOne/PhaseOne.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Eval/ComputeSuccessType.hpp"
#include "../../Output/OutputDirectToFile.hpp"

void NOMAD::PhaseOne::init()
{
    setStepType(NOMAD::StepType::ALGORITHM_PHASE_ONE);
    verifyParentNotNull();

}


void NOMAD::PhaseOne::startImp()
{
    // Temporarily disable solution file (restored in endImp())
    NOMAD::OutputDirectToFile::getInstance()->disableSolutionFile();

    // Default algorithm start
    // See issue #639
    NOMAD::Algorithm::startImp();
    
    // Set up the run parameters to stop once a point that satisfies EB constraints is obtained
    _runParams = std::make_shared<NOMAD::RunParameters>(*_runParams);
    _runParams->setAttributeValue("STOP_IF_PHASE_ONE_SOLUTION", true);
    auto evcParams = NOMAD::EvcInterface::getEvaluatorControl()->getEvaluatorControlGlobalParams();
    _runParams->checkAndComply(evcParams, _pbParams);

    // Setup Mads
    _madsStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::MadsStopType>>();
    _mads = std::make_shared<NOMAD::Mads>(this, _madsStopReasons, _runParams, _pbParams, false /*false: barrier is not initialized from cache*/);
}


bool NOMAD::PhaseOne::runImp()
{
    bool madsSuccess = false;

    auto evc = NOMAD::EvcInterface::getEvaluatorControl();

    auto previousComputeType = evc->getComputeType();
    
    // Override default STANDARD compute type
    if (previousComputeType != NOMAD::ComputeType::STANDARD)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Cannot change compute type to PHASE_ONE if default compute type is not standard");
    }
        
    evc->setComputeType(NOMAD::ComputeType::PHASE_ONE);

    // Run Mads on Phase One.
    _mads->start();
    madsSuccess = _mads->run();
    _mads->end();

    evc->setComputeType(previousComputeType);
    evc->resetBestIncumbent(-1); // Reset for display (-1 for all main threads)
    // Update PhaseOne stop reasons
    if ( !_mads->hasPhaseOneSolution())
    {
        auto phaseOneStopReasons = NOMAD::AlgoStopReasons<NOMAD::PhaseOneStopType>::get(_stopReasons);
        if (!madsSuccess)
        {
            phaseOneStopReasons->set(NOMAD::PhaseOneStopType::MADS_FAIL);
        }
        else
        {
            phaseOneStopReasons->set(NOMAD::PhaseOneStopType::NO_FEAS_PT);
        }
        
        // Phase one has failed to get a solution with EB constraints feasible.
        return false;
    }
    
    return true;
}


void NOMAD::PhaseOne::endImp()
{
    NOMAD::Algorithm::endImp();
    
    // Ensure evaluation of queue will continue
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    evc->restart();
    evc->setLastSuccessfulFeasDir(nullptr);
    evc->setLastSuccessfulInfDir(nullptr);

    // Re-enable writing in Solution file
    NOMAD::OutputDirectToFile::getInstance()->enableSolutionFile();

    auto evalType = evc->getCurrentEvalType(); // Can be BB or SURROGATE
    auto hNormType = evc->getHNormType();
    NOMAD::FHComputeTypeS computeTypeS; // Initialized with default from structure init
    computeTypeS.hNormType = hNormType;
    NOMAD::FHComputeType computeType = {evalType, computeTypeS};
    // Write feasible solution (standard computation, not phase one) in stats and solution
    if (solHasFeas(computeType))
    {
        std::vector<NOMAD::EvalPoint> evalPointList;
        NOMAD::CacheInterface cacheInterface(this);
        size_t numFeas = cacheInterface.findBestFeas(evalPointList, computeType);
        if (numFeas > 0)
        {
            // Evaluation info for output
            NOMAD::StatsInfo info;

            info.setBBO(evalPointList[0].getBBO(NOMAD::EvalType::BB));
            info.setSol(*(evalPointList[0].getX()));

            NOMAD::OutputDirectToFile::Write(info,true,false); // Write in solution (if solution_file exists) but not in history file
        }
    }
}
