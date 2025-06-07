
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/Mads/QuadAndLHSearchMethod.hpp"
#include "../../Algos/Mads/QuadSearchMethod.hpp"
#include "../../Algos/QuadModel/QuadModelAlgo.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Math/LHS.hpp"
#include "../../Output/OutputQueue.hpp"


void NOMAD::QuadAndLHSearchMethod::init()
{
    
    setStepType(NOMAD::StepType::SEARCH_METHOD_QUAD_MODEL);
    verifyParentNotNull();

    const auto parentSearch = getParentStep()->getParentOfType<NOMAD::QuadAndLHSearchMethod*>(false);

    
    
    // For some testing, it is possible that _runParams is null or evaluator control is null
    setEnabled((nullptr == parentSearch)
               && (nullptr !=_runParams)
               && _runParams->getAttributeValue<bool>("QUAD_MODEL_AND_LH_SEARCH")
               &&  (nullptr != EvcInterface::getEvaluatorControl()));
#ifndef USE_SGTELIB
    if (isEnabled())
    {
        OUTPUT_INFO_START
        AddOutputInfo(getName() + " cannot be performed because NOMAD is compiled without sgtelib library");
        OUTPUT_INFO_END
        setEnabled(false);
    }
#endif

#ifdef USE_SGTELIB
    // Check that there is exactly one objective
    if (isEnabled())
    {
        auto nbObj = NOMAD::Algorithm::getNbObj();
        if (0 == nbObj)
        {
            OUTPUT_INFO_START
            AddOutputInfo(getName() + " not performed when there is no objective function");
            OUTPUT_INFO_END
            setEnabled(false);
        }
        else if (nbObj > 1)
        {
            OUTPUT_INFO_START
            AddOutputInfo(getName() + " not performed on multi-objective function");
            OUTPUT_INFO_END
            setEnabled(false);
        }

        auto modelDisplay = _runParams->getAttributeValue<std::string>("QUAD_MODEL_DISPLAY");
        _displayLevel = modelDisplay.empty()
                            ? NOMAD::OutputLevel::LEVEL_DEBUGDEBUG
                            : NOMAD::OutputLevel::LEVEL_INFO;
    }
#endif
}


bool NOMAD::QuadAndLHSearchMethod::runImp()
{
    if ( isEnabled() )
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"QuadAndLHSearchMethod: not fully implemented.");
    }
    
    bool foundBetter = false;
#ifdef USE_SGTELIB
    // The trial points are generated for a feasible frame center and an infeasible one.
    if ( ! _stopReasons->checkTerminate() )
    {
        
        auto nbConsecutiveFail = getSuccessStats().getStatsNbConsecutiveFail();
        
        NOMAD::EvalPointSet trialPts;
        
        if(nbConsecutiveFail > 3) // ChT For testing
        {
    
            auto madsIteration = getParentOfType<MadsIteration*>();
            
            // MegaIteration's barrier member is already in sub dimension.
            auto bestX = madsIteration->getMegaIterationBarrier()->getCurrentIncumbentFeas();
            
            if (nullptr == bestX)
            {
                bestX = madsIteration->getMegaIterationBarrier()->getCurrentIncumbentInf();
            }
            if (nullptr == bestX)
            {
                return false;
            }
            
            auto n = bestX->size();
            
            auto frameSize = madsIteration->getMesh()->getDeltaFrameSize();
            frameSize *= 100;
            
            // LH sampling around best feas and best infeasible.
            NOMAD::ArrayOfDouble lowerBound = *(bestX->getX()) - frameSize ;
            auto upperBound = *(bestX->getX()) + frameSize ;

            NOMAD::Double scaleFactor = sqrt(-log(NOMAD::DEFAULT_EPSILON));
            // Apply Latin Hypercube algorithm (provide frameCenter, deltaFrameSize, and scaleFactor for updating bounds)
            NOMAD::LHS lhs(n, 3*n, lowerBound, upperBound, *bestX, frameSize, scaleFactor);
            auto pointVector = lhs.Sample();
            
            for (const auto& point: pointVector )
            {
                NOMAD::EvalPoint evalPoint(point);
                evalPoint.setPointFrom(std::make_shared<NOMAD::EvalPoint>(*bestX), NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this)); // !!! Point from is a copy of frame center
                evalPoint.addGenStep(getStepType());
                insertTrialPoint(evalPoint);
            }
            // TODO deactivate opportunism
            if ( ! _stopReasons->checkTerminate() )
            {
                foundBetter = evalTrialPoints(this);
            }
            // TODO tune quad model search
            
        }
        
        // QuadSearchMethod
        auto quadSearch             = std::make_shared<NOMAD::QuadSearchMethod>(this);
        quadSearch->generateTrialPoints();
        
        trialPts = quadSearch->getTrialPoints();

        for (auto evalPoint: trialPts )
        {
            evalPoint.addGenStep(getStepType());
            insertTrialPoint(evalPoint);
        }

        if ( ! _stopReasons->checkTerminate() )
        {
            foundBetter = evalTrialPoints(this);
        }

        // From IterationUtils. Update megaIteration barrier.
        postProcessing();
    }
#endif
    //TODO
    return foundBetter;
    
}
void NOMAD::QuadAndLHSearchMethod::generateTrialPointsFinal()
{
    
    // TODO
    
    
}


 // end generateTrialPoints
