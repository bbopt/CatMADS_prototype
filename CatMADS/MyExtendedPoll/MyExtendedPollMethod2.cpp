
#include "Algos/Mads/Mads.hpp"
#include "Algos/Mads/MadsIteration.hpp"
#include "Algos/EvcInterface.hpp"
#include "Output/OutputQueue.hpp"

#include "MyExtendedPollMethod2.hpp"
#include "MySimpleMads.hpp"
#include "../CatMADS/CatMADS.hpp"  // for global variables in CatMADS


void MyExtendedPollMethod2::init()
{
    // Query the enabling parameter here
    auto mads = dynamic_cast<const NOMAD::Mads*>(NOMAD::IterationUtils::_parentStep);
    if ( nullptr == mads)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"ExtendedPollMethod: must have a Mads ancestor");
    }
    
    // For some testing, it is possible that _runParams is null or evaluator control is null
    if ( nullptr != _runParams && nullptr != NOMAD::EvcInterface::getEvaluatorControl() )
    {
        if ( _runParams->getAttributeValue<bool>("MEGA_SEARCH_POLL") )
        {
            throw NOMAD::Exception(__FILE__,__LINE__,"ExtendedPollMethod: does not work for mega search poll");
        }
    }
    
    const auto nbObj = NOMAD::Algorithm::getNbObj();
    if (nbObj > 1)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"ExtendedPollMethod: does not work for multi objective");
    }
    
    setEnabled(true);
    
}


bool MyExtendedPollMethod2::runImp()
{
    if (nullptr == _iterAncestor )
    {
        throw NOMAD::Exception(__FILE__,__LINE__,getName() + " must have an Iteration ancestor.");
    }
    
    auto pollTrialPoints = static_cast<NOMAD::MadsIteration*>(_iterAncestor)->getPollTrialPoints();
    
    // Retrieve incubment solutions for opportunistic stop
    auto bestFeasible = _iterAncestor->getMegaIterationBarrier()->getCurrentIncumbentFeas();
    auto bestInfeasible = _iterAncestor->getMegaIterationBarrier()->getCurrentIncumbentInf();
    NOMAD::FHComputeType computeType = NOMAD::defaultFHComputeType;


    bool success = false;
    for(const auto & pp: pollTrialPoints)
    {

        // Only check poll points from categorical poll
        // If the point is feasible, compare with best feasible value 
        // Otherwise the point is infeasible and compare with best infeasible value 
        // TODO: the parameter \xi is hardcoded to 0.05; use a setup variable 
        if (nullptr != bestFeasible && pp.getGenStep() == NOMAD::StepType::POLL_METHOD_USER &&
            pp.isFeasible(computeType) && (pp.getF(computeType) < bestFeasible->getF(computeType) + 0.05*(bestFeasible->getF(computeType)).abs()))
        {
            cout << endl;
            cout << "Starting feasible point for extended point" << *pp.getX() << " with " << "f=" << pp.getF(computeType) << " h=" << pp.getH(computeType) << endl;
            //success = runOptim(pp, *bestFeasible, *bestInfeasible);
            success = runOptim(pp, bestFeasible, bestInfeasible);
        }
        // For infeasible points, they must also be within hMax
        else if (nullptr != bestInfeasible && pp.getGenStep() == NOMAD::StepType::POLL_METHOD_USER &&
                !pp.isFeasible(computeType) && (pp.getF(computeType) < bestInfeasible->getF(computeType) + 0.05*(bestInfeasible->getF(computeType)).abs())
                && (pp.getH(computeType) < _iterAncestor->getMegaIterationBarrier()->getHMax()))
        {
            cout << endl;
            cout << "Starting infeasible point for extended point" << *pp.getX() << " with " << " f=" << pp.getF(computeType) << " h=" << pp.getH(computeType) << endl;
            //success = runOptim(pp, *bestInfeasible, *bestInfeasible);
            int a = 1;
            success = runOptim(pp, bestFeasible, bestInfeasible);
        }


        // Stop-loop if we have true, meaning that we had a MEGA_SUCCESS with mySimplePoll (better incumbnet found)
        if (success)
        {
            break;
        }
    }
    
    return success;
}

//bool MyExtendedPollMethod2::runOptim(const NOMAD::EvalPoint & pp, const NOMAD::EvalPoint & refBestFeas, 
//                                     const NOMAD::EvalPoint & refBestInf)
bool MyExtendedPollMethod2::runOptim(const NOMAD::EvalPoint & pp, NOMAD::EvalPointPtr refBestFeas, 
    NOMAD::EvalPointPtr refBestInf)
{
    bool success = false;
    
    // Set specific evaluator control
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    if (evc->getCurrentEvalType() != NOMAD::EvalType::BB)
    {
        return false;
    }

    // Get iteration current frame size
    // Frame size is a more robust criterion:
    //  1 frame size value -> 1 mesh size value
    // but 1 mesh size value -> several frame size value (G-Mesh)
    auto currentIterationFrameSize = _iterAncestor->getMesh()->getdeltaMeshSize();
    
    //
    // Set optim parameters
    //
    auto optPbParams = std::make_shared<NOMAD::PbParameters>(*_pbParams);
    auto optRunParams = std::make_shared<NOMAD::RunParameters>(*_runParams);

    // Set min mesh size.
    optPbParams->setAttributeValue("MIN_FRAME_SIZE", currentIterationFrameSize);
    
    // Set initial frame size.
    // Need to reset initial mesh size.
    // When undefined, initial mesh size is computed from initial frame size
    // in checkAndComply
    optPbParams->setAttributeValue("INITIAL_FRAME_SIZE", currentIterationFrameSize);
    optPbParams->resetToDefaultValue("INITIAL_MESH_SIZE");
    
    // No need for anisotropic mesh changes.
    // Keep the current anisotropy in the mesh/frame
    optRunParams->setAttributeValue("ANISOTROPIC_MESH", false);
    
    // What variables need to be fixed in optim -> categorical variables are in group 1
    auto lvg = _pbParams->getAttributeValue<NOMAD::ListOfVariableGroup>("VARIABLE_GROUP");
    auto it_front = lvg.begin(); // Cat variables are in first group. Otherwise use std::advance(it_front, xxx);
    std::set<size_t>::iterator it_ind;
    
    NOMAD::Point fixedVar(pp.size());
    for (it_ind = it_front->begin() ; it_ind != it_front->end(); it_ind++)
    {
        fixedVar[*it_ind] = pp[*it_ind];
    }
    optPbParams->setAttributeValue("FIXED_VARIABLE", fixedVar);
    
    NOMAD::ArrayOfPoint x0s{*pp.getX()};
    optPbParams->setAttributeValue("X0", x0s);

    // We do not want certain warnings appearing in sub-optimization.
    optPbParams->doNotShowWarnings();
    optPbParams->checkAndComply();
    
    // Check and comply for run params
    auto evcParams = evc->getEvaluatorControlGlobalParams();
    optRunParams->checkAndComply(evcParams, optPbParams);
    
    // Bb output type list is passed to MySimpleMads -> needed for eval
    auto bbot = evc->getCurrentEvalParams()->getAttributeValue<NOMAD::BBOutputTypeList>("BB_OUTPUT_TYPE");
    
    // Stop reason not used for the moment.
    // Could be used to indicate what causes the stop.
    auto madsStopReasons = std::make_shared<NOMAD::AlgoStopReasons<NOMAD::MadsStopType>>();
    
    // Find remaining budget of evaluations to avoid extra evaluations by the Extended Poll
    auto nbEvalsDone = evc->getBbEval();
    auto budgetTotal = _pbParams->getAttributeValue<size_t>("DIMENSION") * nbEvalsPerVariable;
    int  remainingBudget = budgetTotal - nbEvalsDone;


    MySimpleMads mads(this, madsStopReasons, optRunParams, optPbParams, bbot, _my_evaluator ,
                        remainingBudget,
                        refBestFeas, /* pass incumbent for opportunistic stop */
                        refBestInf, /* pass incumbent for opportunistic stop */
                        pp, 
                        _iterAncestor->getMegaIterationBarrier()->getHMax()
                    );
    
    mads.start();
    bool runOk = mads.run();
    mads.end();


    NOMAD::SuccessType succesTypeExtendedPoll = NOMAD::SuccessType::UNDEFINED;

    if (runOk)
    {

        // All trial points tested during this extended poll
        auto allEvaluatedTrialPoints = mads.getAllEvaluatedTrialPoints();

        // Transfer Extended poll points into parent mads barrier
        auto parentBarrier = _iterAncestor->getMegaIterationBarrier();

        
        // V1: retrieve succes 
        /*
        MySuccessType succesTypeMyPoll = mads.getMySuccessType();        
        // A better solution was found 
        if (succesTypeMyPoll == MySuccessType::MEGA_SUCCESS){
            succesTypeExtendedPoll = NOMAD::SuccessType::FULL_SUCCESS;
            
            // We will use this bool to stop the Extended Poll
            success = true;
        }
        else{
            if (hMaxAfterUpdate < hMaxBeforeUpdate){
                succesTypeExtendedPoll = NOMAD::SuccessType::PARTIAL_SUCCESS;
            }
            else{
                succesTypeExtendedPoll = NOMAD::SuccessType::UNSUCCESSFUL;
            }
        }
        */

        NOMAD::FHComputeType computeType = NOMAD::defaultFHComputeType;
        bool fullSuccess = false;
        bool partialSuccess = false;
        for (size_t i = 0 ; i < allEvaluatedTrialPoints.size() ; i++){
            
            NOMAD::SuccessType succesTypeTrialPoint = NOMAD::SuccessType::UNDEFINED;
            if (allEvaluatedTrialPoints[i].isFeasible(computeType))
            {
                auto ep_temp = std::make_shared<NOMAD::EvalPoint>(allEvaluatedTrialPoints[i]);
                succesTypeTrialPoint = parentBarrier->getSuccessTypeOfPoints(ep_temp, nullptr);
            }
            else{
                auto ep_temp = std::make_shared<NOMAD::EvalPoint>(allEvaluatedTrialPoints[i]);
                succesTypeTrialPoint = parentBarrier->getSuccessTypeOfPoints(nullptr, ep_temp);
            }

            if (succesTypeTrialPoint == NOMAD::SuccessType::FULL_SUCCESS){
                // Improvement of an incumbent
                fullSuccess = true;
            }
            if (succesTypeTrialPoint == NOMAD::SuccessType::PARTIAL_SUCCESS){
                partialSuccess = true;
            }
        }
        
        // Update barrier after success type has been determined on all points evaluated in the EP
        parentBarrier->updateWithPoints(allEvaluatedTrialPoints, true, true); 

        // Priority is given to fullSuccess
        if (fullSuccess){
            setSuccessType(NOMAD::SuccessType::FULL_SUCCESS);
            // Stop extended poll for this iteration
            success = true; 
        }
        else if(partialSuccess){
            setSuccessType(NOMAD::SuccessType::PARTIAL_SUCCESS);
        }
        else{
            this->setStepType(NOMAD::StepType::EXTENDED_POLL);
            this->setSuccessType(NOMAD::SuccessType::UNSUCCESSFUL);
        }
       
    }

    return success;
}
