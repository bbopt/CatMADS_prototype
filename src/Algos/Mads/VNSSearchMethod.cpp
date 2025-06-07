
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/Mads/VNSSearchMethod.hpp"
#include "../../Algos/VNSMads/VNS.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Output/OutputQueue.hpp"
//
// Reference: File VNS_Search.cpp in NOMAD 3.9.1
// Author: Christophe Tribes

void NOMAD::VNSSearchMethod::init()
{
    setStepType(NOMAD::StepType::SEARCH_METHOD_VNS_MADS);
    verifyParentNotNull();

    const auto parentSearch = getParentStep()->getParentOfType<NOMAD::VNSSearchMethod*>(false);
    
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    
    bool enabled = false;
    
    // Do not perform if EVAL_SURROGATE_OPTIMIZATION is true (parentSearch ==nullptr)
    // For some testing, it is possible that evaluator control or runParams is null
    if (nullptr != evc && nullptr != _runParams)
    {
        auto currentEvalType = evc->getCurrentEvalType();
        
        // Can be enabled only if no parent step is a VNS search method and current eval type is not MODEL
        enabled = (nullptr == parentSearch) && (currentEvalType != EvalType::MODEL);;
        
        if (enabled)
        {
            enabled = _runParams->getAttributeValue<bool>("VNS_MADS_SEARCH");
            
            // Use of surrogate for VNS
            _VNSUseSurrogate = _runParams->getAttributeValue<bool>("VNS_MADS_SEARCH_WITH_SURROGATE");
            
            if (enabled && _VNSUseSurrogate)
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"VNS_MADS_SEARCH_WITH_SURROGATE and VNS_MADS_SEARCH cannot be both enabled.");
            }
            
            if (_VNSUseSurrogate)
            {
                try
                {
                    // For trying to use surrogate, we need to have a surrogate evaluator registered.
                    evc->setCurrentEvaluatorType(EvalType::SURROGATE);
                }
                catch (NOMAD::Exception &e )
                {
                    std::string error = e.what();
                    error += " VNS_MADS_SEARCH_WITH_SURROGATE is enabled but no registered surrogate evaluator is available.";
                    std::cerr << error << std::endl;
                }
                // Put back the current eval type to BB (cannot be MODEL or SURROGATE at this point)
                evc->setCurrentEvaluatorType(EvalType::BB);
                
            }
        }
    }

    setEnabled(enabled);
        
    if (isEnabled())
    {
        _trigger = _runParams->getAttributeValue<NOMAD::Double>("VNS_MADS_SEARCH_TRIGGER").todouble();
        
        // At first the reference frame center is not defined.
        // We obtain the frame center from the EvaluatorControl. If
        _refFrameCenter = NOMAD::Point();
        
        // Create the VNS algorithm with its own stop reason
        _vnsStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::VNSStopType>>();
        _vnsAlgo = std::make_unique<NOMAD::VNS>(this,
                                                _vnsStopReasons ,
                                                _runParams,
                                                _pbParams);
    }

}

bool NOMAD::VNSSearchMethod::runImp()
{
    bool foundBetter = false;
    
    if (isEnabled())
    {
        auto evc = NOMAD::EvcInterface::getEvaluatorControl();
        
        
        const NOMAD::EvalType bbEvalType = NOMAD::EvalType::BB;
        const NOMAD::EvalType searchEvalType = evc->getCurrentEvalType(); // Can be BB or SURROGATE if the whole optimization is with surrogate
        if (NOMAD::EvalType::MODEL == searchEvalType)
        {
            throw NOMAD::Exception(__FILE__,__LINE__,"VNS search cannot be use MODEL evaluation.");
        }
        
        // check the VNS_trigger criterion:
        // If searchEvalType is SURROGATE, the trigger criterion must compare the surrogate equivalent cost in bb (see EVAL_SURROGATE_COST). If eval surrogate cost is INF, the trigger criterion is always true.
        size_t bbEval = evc->getBbEval();
        if (bbEval == 0 || double(_trialPointStats.getNbEvalsDone(bbEvalType))/ (double)bbEval < _trigger)
        {
            
            EvalPointPtr frameCenter = nullptr;
               
            // Barrier of parent Mads not the same as the VNS Mads suboptimization
            std::shared_ptr<NOMAD::BarrierBase> barrier = nullptr;
            
            // Check that mesh from upper MadsMegaIteration is finer than initial
            auto madsMegaIter = getParentOfType<NOMAD::MadsMegaIteration*>(false);
            auto frameSize = madsMegaIter->getMesh()->getDeltaFrameSize();
            auto initialFrameSize = madsMegaIter->getMesh()->getInitialFrameSize();
            
            // Continue only if current frame size is smaller than initial frame size
            if ( initialFrameSize < frameSize )
            {
                OUTPUT_INFO_START
                AddOutputInfo("Current frame size larger than initial one. Stop VNS Mads Search.");
                OUTPUT_INFO_END
                setSuccessType(NOMAD::SuccessType::UNSUCCESSFUL);
                
                return foundBetter;
            }
            
            // Get barrier from upper MadsMegaIteration, if available.
            if (nullptr != madsMegaIter)
            {
                barrier = madsMegaIter->getBarrier();
            }
            else
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"VNS Mads needs a barrier");
            }
            
            // MegaIteration's barrier member is already in sub dimension.
            auto bestXFeas = barrier->getCurrentIncumbentFeas();
            auto bestXInf  = barrier->getCurrentIncumbentInf();
            
            // Get the frame center for VNS sub optimization
            auto computeType = barrier->getFHComputeType();
    
            if (nullptr != bestXFeas
                && bestXFeas->getF(computeType).isDefined()
                && bestXFeas->getF(computeType) < MODEL_MAX_OUTPUT)
            {
                frameCenter = bestXFeas;
            }
            else if (nullptr != bestXInf
                     && bestXInf->getF(computeType).isDefined()
                     && bestXInf->getF(computeType) < MODEL_MAX_OUTPUT
                     && bestXInf->getH(computeType).isDefined()
                     && bestXInf->getH(computeType) < MODEL_MAX_OUTPUT)
            {
                frameCenter = bestXInf;
            }
            
            
            if ( nullptr != frameCenter )
            {
                if (_VNSUseSurrogate)
                {
                    // Set a null barrier to the VNS algo.
                    // We should no use the evc existing BB type barrier for the initial point surrogate evaluation.
                    // The barrier is updated with the VNS Mads barrier when available.
                    evc->setBarrier(nullptr);
                    evc->setCurrentEvaluatorType(NOMAD::EvalType::SURROGATE);
                }
                
                _vnsAlgo->setEndDisplay(false);
                
                // VNS algo needs a frame center used as initial point for sub-optimization
                _vnsAlgo->setFrameCenter(frameCenter);
                
                // VNS conduct sub-optimization
                _vnsAlgo->start();
                _vnsAlgo->run();
                _vnsAlgo->end();
                
                // TODO we may need to check the _vnsStopReasons.
                
                // Get the success type and update Mads barrier with VNS Mads barrier
                auto vnsBarrier = _vnsAlgo->getBarrier();
                
                if (nullptr == vnsBarrier)
                {
                    throw NOMAD::Exception(__FILE__,__LINE__,"VNS Mads barrier is not available.");
                }
                
                // TODO check that we have the right eval type best feas
                auto vnsBestFeas = vnsBarrier->getCurrentIncumbentFeas();
                auto vnsBestInf = vnsBarrier->getCurrentIncumbentInf();
                
                // If searchEvalType is surrogate perform BB evaluation on the selected point.
                if (_VNSUseSurrogate)
                {
                    if (nullptr != vnsBestFeas)
                    {
                        insertTrialPoint(*vnsBestFeas);
                    }
                    if (nullptr != vnsBestInf)
                    {
                        insertTrialPoint(*vnsBestInf);
                    }
                    evc->setCurrentEvaluatorType(NOMAD::EvalType::BB);
                    return evalTrialPoints(this);
                    
                }
                
                NOMAD::SuccessType success = barrier->getSuccessTypeOfPoints(vnsBestFeas,
                                                                             vnsBestInf);
                setSuccessType(success);
                if (success >= NOMAD::SuccessType::PARTIAL_SUCCESS)
                {
                    foundBetter = true;
                }
                
                
                // Update the barrier
                if ( NOMAD::EvalType::BB == searchEvalType )
                {
                    barrier->updateWithPoints(vnsBarrier->getAllPoints(),
                                              _runParams->getAttributeValue<bool>("FRAME_CENTER_USE_CACHE"),
                                              true /*true: update incumbents and hMax*/);
                }
            }
        }
        else
        {
            OUTPUT_INFO_START
            AddOutputInfo("VNS trigger criterion not met. Stop VNS Mads Search.");
            OUTPUT_INFO_END
        }
    }
    return foundBetter;
}


void NOMAD::VNSSearchMethod::generateTrialPointsFinal()
{
    std::string s;
    NOMAD::EvalPointSet trialPoints;

    throw NOMAD::Exception(__FILE__,__LINE__,"VNS Mads generateTrialPointsFinal() not yet implemented.");
    
    // The trial points of one iteration of VNS are generated (not evaluated).
    // The trial points are obtained by shuffle + mads poll

    // auto madsIteration = getParentOfType<MadsIteration*>();

    /*
    // Note: Use first point of barrier as simplex center.
    NOMAD::VNSSingle singleVNS(this,
                            std::make_shared<NOMAD::EvalPoint>(getMegaIterationBarrier()->getFirstPoint()),
                            madsIteration->getMesh());
    singleVNS.start();
    singleVNS.end();

    // Pass the generated trial pts to this
    const auto& trialPts = singleVNS.getTrialPoints();
    for (const auto& point : trialPts)
    {
        insertTrialPoint(point);
    }
     */

}   // end generateTrialPoints


