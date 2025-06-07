
#include "../../Algos/Mads/VNSmartSearchMethod.hpp"
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/VNSMads/VNS.hpp"
#include "../../Algos/Algorithm.hpp"
#include "../../Eval/Eval.hpp"

void NOMAD::VNSmartAlgoSearchMethod::init()
{
    setStepType(NOMAD::StepType::SEARCH_METHOD_VNSMART_MADS);
    verifyParentNotNull();

    const auto parentSearch = getParentStep()->getParentOfType<NOMAD::VNSmartAlgoSearchMethod*>(false);
    
    // For some testing, it is possible that evaluator control is null
    if (nullptr != NOMAD::EvcInterface::getEvaluatorControl())
    {
        bool bBEval = ( NOMAD::EvcInterface::getEvaluatorControl()->getCurrentEvalType() == EvalType::BB ) ;
        
        // For some testing, it is possible that _runParams is null
        setEnabled((nullptr == parentSearch) && nullptr != _runParams && _runParams->getAttributeValue<bool>("VNSMART_MADS_SEARCH") && bBEval);
    }
    else
    {
        setEnabled(false);
    }
    if (isEnabled())
    {
        // At first the reference frame center is not defined.
        // We obtain the frame center from the EvaluatorControl. If
        _refFrameCenter = NOMAD::Point();
        
        _stopConsFailures = _runParams->getAttributeValue<int>("VNSMART_MADS_SEARCH_THRESHOLD");
        
        // Create the VNS algorithm with its own stop reason
        _vnsStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::VNSStopType>>();
        _vnsAlgo = std::make_unique<NOMAD::VNS>(this,
                                                _vnsStopReasons ,
                                                _runParams,
                                                _pbParams);
    }
    
}


bool NOMAD::VNSmartAlgoSearchMethod::runImp()
{
    bool foundBetter = false;
    
    
    if (isEnabled())
    {
        // Collect the information needed from the current iteration
        auto madsMegaIter = getParentOfType<NOMAD::MadsMegaIteration*>(false);
        auto nbConsecutiveFail = madsMegaIter->getConstSuccessStats().getStatsNbConsecutiveFail();
        
        if (static_cast<int>(nbConsecutiveFail) >= _stopConsFailures) // If the criterion is met, VNS MADS is used for the search step
        {
            EvalPointPtr frameCenter = nullptr;
            
            // Barrier of parent Mads not the same as the VNS Mads suboptimization
            std::shared_ptr<NOMAD::BarrierBase> barrier = nullptr;
            
            // Check that mesh from upper MadsMegaIteration is finer than initial
            auto megaIter = getParentOfType<NOMAD::MegaIteration*>(false);
            if (megaIter == nullptr)
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"VNS Mads needs a MegaIteration parent");
            }
            auto mesh = megaIter->getMesh();
            if (mesh == nullptr)
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"VNS Mads needs a mesh");
            }
            auto frameSize = megaIter->getMesh()->getDeltaFrameSize();
            auto initialFrameSize = megaIter->getMesh()->getInitialFrameSize();

            // Get barrier from upper MadsMegaIteration, if available.
            barrier = megaIter->getBarrier();
            if (barrier == nullptr)
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
                
                _vnsAlgo->setEndDisplay(false);

                // VNS algo needs a frame center used as initial point for sub-optimization
                _vnsAlgo->setFrameCenter(frameCenter);

                // VNS conduct sub-optimization
                _vnsAlgo->start();
                _vnsAlgo->run();
                _vnsAlgo->end();
                
                // Get the success type and update Mads barrier with VNS Mads barrier
                auto vnsBarrier = _vnsAlgo->getBarrier();
                
                if (nullptr != vnsBarrier)
                {
                    auto vnsBestFeas = vnsBarrier->getCurrentIncumbentFeas();
                    auto vnsBestInf = vnsBarrier->getCurrentIncumbentInf();
                    NOMAD::SuccessType success = barrier->getSuccessTypeOfPoints(vnsBestFeas,
                                                                                 vnsBestInf);
                    
                    setSuccessType(success);
                    if (success >= NOMAD::SuccessType::PARTIAL_SUCCESS)
                    {
                        foundBetter = true;
                    }
                    
                    // Update the barrier
                    barrier->updateWithPoints(vnsBarrier->getAllPoints(),
                                              _runParams->getAttributeValue<bool>("FRAME_CENTER_USE_CACHE"),
                                                                    true /* true: update incumbents and hMax */);
                    
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


void NOMAD::VNSmartAlgoSearchMethod::generateTrialPointsFinal()
{
    std::string s;
    NOMAD::EvalPointSet trialPoints;

    throw NOMAD::Exception(__FILE__,__LINE__,"VNS Mads generateTrialPointsFinal() not yet implemented.");

}
