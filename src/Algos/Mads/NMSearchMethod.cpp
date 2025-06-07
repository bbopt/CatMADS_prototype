
#include "../../Algos/Mads/NMSearchMethod.hpp"
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/NelderMead/NMAllReflective.hpp"

void NOMAD::NMSearchMethod::init()
{
    
    // For some testing, it is possible that _runParams is null or evaluator control is null
    bool nmSearch = false;
    if ( nullptr != _runParams && nullptr != NOMAD::EvcInterface::getEvaluatorControl() )
    {
        if ( _runParams->getAttributeValue<bool>("MEGA_SEARCH_POLL") )
        {
            setStepType(NOMAD::StepType::SEARCH_METHOD_NM);
        }
        else
        {
            setStepType(NOMAD::StepType::ALGORITHM_NM);
        }
        nmSearch = _runParams->getAttributeValue<bool>("NM_SEARCH");
    }
    setEnabled(nmSearch);
    
    if (isEnabled())
    {
        const auto nbObj = NOMAD::Algorithm::getNbObj();
        if (nbObj > 1)
        {
            OUTPUT_INFO_START
            AddOutputInfo(getName() + " not performed on multi-objective function");
            OUTPUT_INFO_END
            setEnabled(false);
            return;
        }
    }
    
    if (nmSearch)
    {
        // Set the lap counter
        const auto nmFactor = _runParams->getAttributeValue<size_t>("NM_SEARCH_MAX_TRIAL_PTS_NFACTOR");
        const auto dim = _pbParams->getAttributeValue<size_t>("DIMENSION");
        if (nmFactor < NOMAD::INF_SIZE_T)
        {
            NOMAD::EvcInterface::getEvaluatorControl()->setLapMaxBbEval( dim*nmFactor );
        }
        
        // NM is an algorithm with its own stop reasons.
        _nmStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::NMStopType>>();
        
        // Create the NM algorithm with its own stop reason
        _nm = std::make_unique<NOMAD::NM>(this,
                                              _nmStopReasons ,
                                              _runParams,
                                              _pbParams);
        
    }
}


bool NOMAD::NMSearchMethod::runImp()
{
   
    _nm->setEndDisplay(false);

    _nm->start();
    bool foundBetter = _nm->run();
    _nm->end();

    // Maybe use _nmStopReason to update parent algorithm
    
    return foundBetter;
}


void NOMAD::NMSearchMethod::generateTrialPointsFinal()
{
    // The trial points of one iteration of NM reflective steps are generated (not evaluated).
    // The trial points are Reflect, Expansion, Inside and Outside Contraction NM points

    auto madsIteration = getParentOfType<MadsIteration*>();

    // Note: Use first point of barrier as simplex center.
    // See issue #392
    NOMAD::NMAllReflective allReflective(this,
                            getMegaIterationBarrier()->getFirstPoint(),
                            madsIteration->getMesh());
    allReflective.start();
    allReflective.end();

    // Pass the generated trial pts to this
    const auto& trialPtsNM = allReflective.getTrialPoints();
    for (const auto& point : trialPtsNM)
    {
        insertTrialPoint(point);
    }

}
