
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/MadsUpdate.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Algos/SSDMads/SSDMads.hpp"
#include "../../Algos/SSDMads/SSDMadsMegaIteration.hpp"
#include "../../Eval/ComputeSuccessType.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Type/DirectionType.hpp"


void NOMAD::SSDMadsMegaIteration::init()
{
    setStepType(NOMAD::StepType::MEGA_ITERATION);
}


void NOMAD::SSDMadsMegaIteration::startImp()
{
    // Update manager mesh and barrier.
    NOMAD::MadsUpdate update( this );
    update.start();
    update.run();
    update.end();
    
    // Reset the random pickup (put back n to its initial value)
    _randomPickup.reset();

    // Verify mesh stop conditions.
    _mainMesh->checkMeshForStopping( _stopReasons );

    OUTPUT_DEBUG_START
    AddOutputDebug("Mesh Stop Reason: " + _stopReasons->getStopReasonAsString());
    OUTPUT_DEBUG_END
    if ( ! _stopReasons->checkTerminate() )
    {
        auto bestEvalPoint = _barrier->getCurrentIncumbentFeas();

        if (bestEvalPoint == nullptr)
            bestEvalPoint  = _barrier->getCurrentIncumbentInf();

        if (bestEvalPoint == nullptr)
            throw NOMAD::Exception(__FILE__, __LINE__, "No best eval point");

        auto nbMadsSubproblem = _runParams->getAttributeValue<size_t>("SSD_MADS_NB_SUBPROBLEM");

        for (size_t nbMads = 0; nbMads < nbMadsSubproblem; nbMads++)
        {
            // The stop reasons for a subproblem mads
            auto stopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::MadsStopType>>();

            auto subProblemPbParams = std::make_shared<NOMAD::PbParameters>(*_pbParams);

            auto subProblemRunParams = std::make_shared<NOMAD::RunParameters>(*_runParams);

            bool isPollsterWorker = (nbMads == 0);

            setupSubproblemParams (subProblemPbParams, subProblemRunParams, *(bestEvalPoint->getX()), isPollsterWorker );

            // Second check, first check is done during setup
            subProblemPbParams->checkAndComply();

            auto evcParams = NOMAD::EvcInterface::getEvaluatorControl()->getEvaluatorControlGlobalParams();

            subProblemRunParams->checkAndComply(evcParams, subProblemPbParams);

            auto madsOnASubProblem = std::make_shared<NOMAD::Mads>(this , stopReasons, subProblemRunParams, subProblemPbParams , false /*false: Barrier not initialized from cache */);
            _madsList.push_back(madsOnASubProblem);
        }


        OUTPUT_INFO_START
        AddOutputInfo(getName() + " has " + std::to_string(nbMadsSubproblem) + " subproblem mads.");
        OUTPUT_INFO_END
        
        // Default mega iteration start tasks
        // See issue #639
        NOMAD::MegaIteration::startImp();
        
    }
}



bool NOMAD::SSDMadsMegaIteration::runImp()
{

    NOMAD::SuccessType subPbSuccess = SuccessType::UNDEFINED;

    std::string s;

    if ( _stopReasons->checkTerminate() )
    {
        OUTPUT_DEBUG_START
        s = "SSDMadsMegaIteration: stopReason = " + _stopReasons->getStopReasonAsString() ;
        AddOutputDebug(s);
        OUTPUT_DEBUG_END
        return false;
    }

    if (_madsList.empty())
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "No mads on subproblem to run");
    }

    // Get Mads ancestor to call terminate(k)
    auto* ssdmads = getParentOfType<NOMAD::SSDMads*>();
    if (nullptr == ssdmads)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "SSDMads Iteration without SSDMads ancestor");
    }

    // Reset counter for bb eval in subproblem for this sub-optimization
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    evc->resetBbEvalInSubproblem();

    for (size_t i = 0; i < _madsList.size(); i++)
    {
        if (_stopReasons->checkTerminate()
            || evc->testIf(NOMAD::EvalMainThreadStopType::OPPORTUNISTIC_SUCCESS)
            || ssdmads->terminate(i))
        {
            break;
        }

        // downcast from Iteration to MadsIteration
        std::shared_ptr<NOMAD::Mads> madsOnSubPb = std::dynamic_pointer_cast<NOMAD::Mads>(_madsList[i]);

        if (madsOnSubPb == nullptr)
        {
            throw NOMAD::Exception(__FILE__, __LINE__, "Invalid shared pointer cast");
        }

        madsOnSubPb->start();

        bool iterSuccessful = madsOnSubPb->run();   // Is this iteration successful
        madsOnSubPb->end();

        // Compute success
        NOMAD::EvalPointPtr newBestFeas,newBestInf;

        // Use mega iteration barrier to get the reference best points (should work for opportunistic or not)
        auto refBestFeas = _barrier->getRefBestFeas();
        auto refBestInf = _barrier->getRefBestInf();

        // Set the iter success based on best feasible and best infeasible points found compared to initial point.
        if (iterSuccessful && (nullptr != refBestFeas || nullptr != refBestInf))
        {
            // Use the mads barrier to access the new best points
            auto barrier = madsOnSubPb->getMegaIterationBarrier();
            if (nullptr != barrier)
            {
                newBestFeas = barrier->getCurrentIncumbentFeas();
                newBestInf = barrier->getCurrentIncumbentInf();
                
                auto computeType = barrier->getFHComputeType();

                // Compute success
                // Get which of newBestFeas and newBestInf is improving
                // the solution. Check newBestFeas first.
                NOMAD::ComputeSuccessType computeSuccess(computeType);
                subPbSuccess = computeSuccess(newBestFeas, refBestFeas);
                if (subPbSuccess >= NOMAD::SuccessType::PARTIAL_SUCCESS)
                {
                    // newBestFeas is the improving point.
                    OUTPUT_DEBUG_START
                    // Output Warning: When using '\n', the computed indentation for the
                    // Step will be ignored. Leaving it like this for now. Using an
                    // OutputInfo with AddMsg() would resolve the output layout.
                    s = "Update: improving feasible point";
                    if (refBestFeas)
                    {
                        s += " from\n    " + refBestFeas->display() + "\n";
                    }
                    s += " to " + newBestFeas->display();
                    AddOutputDebug(s);
                    OUTPUT_DEBUG_END
                }
                else
                {
                    // Check newBestInf
                    NOMAD::SuccessType success2 = computeSuccess(newBestInf, refBestInf);
                    if (success2 > subPbSuccess)
                    {
                        subPbSuccess = success2;
                    }
                    if (subPbSuccess >= NOMAD::SuccessType::PARTIAL_SUCCESS)
                    {
                        OUTPUT_DEBUG_START
                        s = "Update: improving infeasible point";
                        if (refBestInf)
                        {
                            s+= " from\n    " + refBestInf->display() + "\n";
                        }
                        s += " to " + newBestInf->display();
                        AddOutputDebug(s);
                        OUTPUT_DEBUG_END
                    }
                }
                if (subPbSuccess == NOMAD::SuccessType::UNSUCCESSFUL)
                {
                    OUTPUT_DEBUG_START
                    s = "Update: no success found";
                    AddOutputDebug(s);
                    OUTPUT_DEBUG_END
                }
            }
        }
        if (subPbSuccess > _success)
        {
            _success = subPbSuccess;
        }

        //
        // Transfer the Mads subproblem barrier into the mega iteration barrier
        //
        // The fixed variables of the mads subproblem
        auto fixedVariable = NOMAD::SubproblemManager::getInstance()->getSubFixedVariable((madsOnSubPb.get()));
        auto evalPointList = madsOnSubPb->getMegaIterationBarrier()->getAllPoints();
        // Convert into full dimension
        NOMAD::convertPointListToFull(evalPointList, fixedVariable);
        _barrier->updateWithPoints(evalPointList, true, true /* true: update incumbents and hMax */);

        // Reset counter will reset the EvalMainThreadStopReason if the max bb is reached for this sub optimization
        evc->resetBbEvalInSubproblem();

        // Set the stop reason for opportunistic success of a subproblem mads
        const bool opportunisticItStop = _runParams->getAttributeValue<bool>("SSD_MADS_ITER_OPPORTUNISTIC");
        if (opportunisticItStop && (_success == NOMAD::SuccessType::FULL_SUCCESS))
        {
            evc->setStopReason(NOMAD::getThreadNum(), NOMAD::EvalMainThreadStopType::OPPORTUNISTIC_SUCCESS);
        }


//      Display MegaIteration's stop reason
        if (_stopReasons->checkTerminate())
        {
            OUTPUT_DEBUG_START
            s = getName() + " stop reason set to: " + _stopReasons->getStopReasonAsString();
            AddOutputDebug(s);
            OUTPUT_DEBUG_END
        }

        if (getUserInterrupt())
        {
            hotRestartOnUserInterrupt();
        }

    }

    // return true if we have a partial or full success.
    return (_success >= NOMAD::SuccessType::PARTIAL_SUCCESS);
}


void NOMAD::SSDMadsMegaIteration::setupSubproblemParams ( std::shared_ptr<NOMAD::PbParameters> & subProblemPbParams, std::shared_ptr<NOMAD::RunParameters> & subProblemRunParams, const NOMAD::Point & bestPoint, bool isPollster)
{
    auto mainFrameSize = _mainMesh->getDeltaFrameSize();

    subProblemPbParams->doNotShowWarnings();
    if (isPollster)
    {
        subProblemRunParams->setAttributeValue("DIRECTION_TYPE", NOMAD::DirectionType::SINGLE );
        subProblemRunParams->setAttributeValue("MAX_ITERATIONS", 1);
        subProblemPbParams->setAttributeValue("INITIAL_FRAME_SIZE", mainFrameSize);

        return;

    }


    auto initialFrameSize = _mainMesh->getDeltaFrameSizeCoarser();
    subProblemPbParams->setAttributeValue("INITIAL_FRAME_SIZE", initialFrameSize);

////    // Check to get the initial frame size.
//    subProblemPbParams->checkAndComply();
//
//    auto  initialFrameSize = subProblemPbParams->getAttributeValue<NOMAD::ArrayOfDouble>("INITIAL_FRAME_SIZE");
//
//    initialFrameSize *= 0.1;
//    initialFrameSize = initialFrameSize + mainFrameSize;
//    initialFrameSize *= 0.909091;
//    subProblemPbParams->setAttributeValue("INITIAL_FRAME_SIZE", initialFrameSize);


    // The main frame size is used as minFrameSize for the subproblem.
    // Initial and min must be compatible -> adjust.
    for (size_t i =0 ; i < initialFrameSize.size() ; i++ )
    {
        if (initialFrameSize[i] < mainFrameSize[i])
        {
            OUTPUT_INFO_START
            AddOutputInfo("Set initial frame size to main frame size.");
            OUTPUT_INFO_END
            subProblemPbParams->setAttributeValue("INITIAL_FRAME_SIZE", mainFrameSize);
            break;
        }
    }


    // The fixed variables of the subproblem are set to the value of best point, the remaining variables are undefined.
    const auto nbVariablesInSubproblem = _runParams->getAttributeValue<size_t>("SSD_MADS_NB_VAR_IN_SUBPROBLEM"); // Number of variables in Subproblem
    if (nbVariablesInSubproblem >= bestPoint.size())
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Nb variables in subproblem cannot greater or equal to the overall dimension of the problem.");
    }
    
    if (_runParams->getAttributeValue<bool>("SSD_MADS_RESET_VAR_PICKUP_SUBPROBLEM") )
    {
        _randomPickup.reset();
    }
    NOMAD::Point fixedVariables = bestPoint;
    for (size_t k = 0; k < nbVariablesInSubproblem ; k++ )
    {
        fixedVariables[_randomPickup.pickup()] = NOMAD::Double();
    }

    subProblemPbParams->setAttributeValue("FIXED_VARIABLE",fixedVariables);
    subProblemPbParams->setAttributeValue("X0",bestPoint);
    subProblemPbParams->setAttributeValue("MIN_FRAME_SIZE",mainFrameSize);

}
