
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/Mads/SinglePollMethod.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Type/LHSearchType.hpp"
#include "../../Util/fileutils.hpp"

// VNS specific
#include "../../Algos/VNSMads/VNS.hpp"


void NOMAD::VNS::init()
{
    /*
    if ( _runParams->getAttributeValue<bool>("MEGA_SEARCH_POLL") )
    {
        _name += " One Iteration";
    }
     */
    setStepType(NOMAD::StepType::ALGORITHM_VNS_MADS);

}


void NOMAD::VNS::startImp()
{
    if (nullptr == _frameCenter)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"VNS Mads needs a frame center");
    }
    
    // Default algorithm start
    // See issue #639
    NOMAD::Algorithm::startImp();

    // Setup Mads
    _madsStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::MadsStopType>>();

    // Increment the neighborhood parameter
    _neighParameter ++;
    
}

bool NOMAD::VNS::runImp()
{
    _algoSuccessful = false;

    auto VNSStopReasons = NOMAD::AlgoStopReasons<NOMAD::VNSStopType>::get(_stopReasons);

    if ( _stopReasons->checkTerminate() )
    {
        return _algoSuccessful;
    }

    if (_runParams->getAttributeValue<bool>("VNS_MADS_OPTIMIZATION"))
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"VNS_MADS_OPTIMIZATION not yet implemented");
    }

    // Get the parent Mads Mega iteration and its associated mesh
    auto parentMadsMegaIter = getParentOfType<NOMAD::MadsMegaIteration*>(false);
    if (nullptr == parentMadsMegaIter)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"VNS Mads needs a MadsMegaIteration");
    }

    // Shaking direction: use single poll method
    NOMAD::SinglePollMethod pollMethod(this,_frameCenter);
    std::list<NOMAD::Direction> scaledDirection = pollMethod.generateFullSpaceScaledDirections(false, parentMadsMegaIter->getMesh());

    // Get the single direction
    NOMAD::Direction dir = scaledDirection.front();
    if (!dir.isDefined())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"VNS: single scaled direction not defined");
    }

    // Multiply shake direction by VNS neighborhood parameter
    dir *= _neighParameter;

    OUTPUT_INFO_START
    AddOutputInfo("Shaking direction: " + dir.display());
    OUTPUT_INFO_END

    // shaking: the perturbation is tried twice with dir and -dir
    //          (in case x == x + dir after snapping)
    NOMAD::Point shakePoint = *(_frameCenter->getX()) + dir; // pun intended;
    shakePoint.snapToBounds(_pbParams->getAttributeValue<ArrayOfDouble>("LOWER_BOUND"),_pbParams->getAttributeValue<ArrayOfDouble>("UPPER_BOUND"));
    for ( int nbt = 0 ; nbt < 2 ; ++nbt )
    {
        if ( shakePoint == *(_frameCenter->getX()))
        {
            // no third try: the search fails
            if ( nbt == 1 )
            {
                OUTPUT_INFO_START
                AddOutputInfo("VNS: Shaking failed");
                OUTPUT_INFO_END

                VNSStopReasons->set(NOMAD::VNSStopType::SHAKING_FAILED);

                return _algoSuccessful;
            }

            // 2nd try (-dir instead of dir):
            shakePoint =  *(_frameCenter->getX()) - dir;
            shakePoint.snapToBounds(_pbParams->getAttributeValue<ArrayOfDouble>("LOWER_BOUND"),_pbParams->getAttributeValue<ArrayOfDouble>("UPPER_BOUND"));
        }
    }

    // Generate base point (X0 for Mads)
    auto evalPoint = NOMAD::EvalPoint(shakePoint);

    // Meta information on eval point
    evalPoint.setPointFrom(_frameCenter, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
    evalPoint.addGenStep(getStepType());

    // Get the current Mads frame size to be used as min frame size for sub-optimization
    NOMAD::ArrayOfDouble currentMadsFrameSize = parentMadsMegaIter->getMesh()->getDeltaFrameSize();

    setupPbParameters(shakePoint,currentMadsFrameSize);
    setupRunParameters();
    
    // In case we are doing a VNS using Surrogate (VNS_MADS_SEARCH_WITH_SURROGATE) we must not use the surrogate for the evaluation queue sort.
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    auto evalType = evc->getCurrentEvalType();
    auto evalSortType = evc->getEvalSortType();
    bool evalSortTypeChanged = false;
    if (evalType == NOMAD::EvalType::SURROGATE && evalSortType == NOMAD::EvalSortType::SURROGATE)
    {  // Force eval sort type to DIR_LAST_SUCCESS when doing VNS with Surrogate
        // Quad model is probably not adapted for this task because
        // the quad model is built using the true BB.
        // Note: Still, surrogate can be used to sort trial points before BB evaluation.
        evc->setEvalSortType(NOMAD::EvalSortType::DIR_LAST_SUCCESS);
        evalSortTypeChanged = true;
    }

    NOMAD::Mads mads(this, _madsStopReasons, _optRunParams, _optPbParams, false /*false: Barrier not initialized from cache */ );

    // Run Mads.
    mads.start();
    mads.run();
    mads.end();
    
    if ( _madsStopReasons->testIf(NOMAD::MadsStopType::X0_FAIL) )
    {
        VNSStopReasons->set(NOMAD::VNSStopType::X0_FAILED);
    }
    else
    {
        _barrier = mads.getMegaIterationBarrier();
        _algoSuccessful = true;
    }
    
    // Reset the eval sort type to surrogate if it has been changed
    if (evalSortTypeChanged)
    {
        evc->setEvalSortType(NOMAD::EvalSortType::SURROGATE);
    }

    _termination->start();
    _termination->run();
    _termination->end();

    return _algoSuccessful;
}


void NOMAD::VNS::endImp()
{
    // See issue #639
    NOMAD::Algorithm::endImp();
}


void NOMAD::VNS::setupRunParameters()
{
    _optRunParams = std::make_shared<NOMAD::RunParameters>(*_runParams);

    _optRunParams->setAttributeValue("MAX_ITERATIONS", INF_SIZE_T);

    // VNS do not perform VNS search
    _optRunParams->setAttributeValue("VNS_MADS_SEARCH", false);
    _optRunParams->setAttributeValue("VNS_MADS_SEARCH_WITH_SURROGATE",false);

    // No LH search
    _optRunParams->setAttributeValue("LH_SEARCH", NOMAD::LHSearchType("0 0"));

    // No callbacks for mads iterations in VNS optimization : for the Restart_VNS example
    _optRunParams->setAttributeValue("USER_CALLS_ENABLED", false);
    
    auto vnsFactor = _runParams->getAttributeValue<size_t>("VNS_MADS_SEARCH_MAX_TRIAL_PTS_NFACTOR");
    auto dim = _pbParams->getAttributeValue<size_t>("DIMENSION");
    if (vnsFactor < NOMAD::INF_SIZE_T)
    {
        NOMAD::EvcInterface::getEvaluatorControl()->setLapMaxBbEval( dim*vnsFactor );
    }

    auto evcParams = NOMAD::EvcInterface::getEvaluatorControl()->getEvaluatorControlGlobalParams();
    
    _optRunParams->checkAndComply(evcParams, _optPbParams);
}


void NOMAD::VNS::setupPbParameters(const NOMAD::Point & center, const NOMAD::ArrayOfDouble & currentMadsFrameSize)
{
    _optPbParams = std::make_shared<NOMAD::PbParameters>(*_pbParams);

    // Reset initial mesh and frame sizes
    // The initial mesh and frame sizes will be calculated from bounds and X0
    _optPbParams->resetToDefaultValue("INITIAL_MESH_SIZE");
    _optPbParams->resetToDefaultValue("INITIAL_FRAME_SIZE");

    // set min frame size (min mesh size will be updated)
    _optPbParams->resetToDefaultValue("MIN_MESH_SIZE");
    _optPbParams->resetToDefaultValue("MIN_FRAME_SIZE");
    _optPbParams->setAttributeValue("MIN_FRAME_SIZE", currentMadsFrameSize);

    NOMAD::ArrayOfPoint x0s{center};
    _optPbParams->setAttributeValue("X0", x0s);

    // We do not want certain warnings appearing in sub-optimization.
    _optPbParams->doNotShowWarnings();

    _optPbParams->checkAndComply();

}

void NOMAD::VNS::setFrameCenter(const EvalPointPtr& frameCenter)
{
    _frameCenter=frameCenter ;
 
    if ( !_refFrameCenter.isDefined() || *(frameCenter->getX()) != _refFrameCenter)
    {
        _neighParameter = 0.0;
        _refFrameCenter = *(frameCenter->getX());
    }
}
