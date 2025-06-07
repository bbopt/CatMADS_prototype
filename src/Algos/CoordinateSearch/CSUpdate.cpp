
#include "../../Algos/CacheInterface.hpp"
#include "../../Algos/CoordinateSearch/CSMegaIteration.hpp"
#include "../../Algos/CoordinateSearch/CSUpdate.hpp"
#include "../../Algos/CoordinateSearch/CSMesh.hpp"
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Eval/ComputeSuccessType.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::CSUpdate::init()
{
    setStepType(NOMAD::StepType::UPDATE);
    verifyParentNotNull();

    auto megaIter = getParentOfType<NOMAD::CSMegaIteration*>();
    if (nullptr == megaIter)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Error: An instance of class CSUpdate must have a MegaIteration among its ancestors");
    
    }

}

std::string NOMAD::CSUpdate::getName() const
{
    return getAlgoName() + NOMAD::stepTypeToString(_stepType);
}


bool NOMAD::CSUpdate::runImp()
{
    // megaIter barrier is already in subproblem.
    // So no need to convert refBestFeas and refBestInf
    // from full dimension to subproblem.
    auto megaIter = getParentOfType<NOMAD::CSMegaIteration*>();
    auto barrier = megaIter->getBarrier();
    
    auto computeType = barrier->getFHComputeType();
    NOMAD::EvalType evalType = computeType.evalType;
    NOMAD::FHComputeTypeS computeTypeS = computeType.fhComputeTypeS;
    
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    
    auto mesh = megaIter->getMesh();
    std::string s;  // for output

    OUTPUT_DEBUG_START
    s = "Running " + getName() + ". Barrier: ";
    AddOutputDebug(s);
    std::vector<std::string> vs = barrier->display(4);
    for (const auto & si : vs)
    {
        AddOutputDebug(si);
    }
    OUTPUT_DEBUG_END

    // Barrier is already updated from previous steps.
    // Get the best feasible and infeasible reference points, and then update
    // reference values.
    auto refBestFeas = barrier->getRefBestFeas();
    auto refBestInf  = barrier->getRefBestInf();

    barrier->updateRefBests();

    NOMAD::EvalPointPtr newBestFeas = barrier->getCurrentIncumbentFeas();
    NOMAD::EvalPointPtr newBestInf  = barrier->getCurrentIncumbentInf();

    if (nullptr != refBestFeas || nullptr != refBestInf)
    {
        // Compute success
        // Get which of newBestFeas and newBestInf is improving
        // the solution. Check newBestFeas first.
        NOMAD::ComputeSuccessType computeSuccess(computeType);
        NOMAD::EvalPointPtr newBest;
        NOMAD::SuccessType success = computeSuccess(newBestFeas, refBestFeas);
        
        if (success >= NOMAD::SuccessType::PARTIAL_SUCCESS)
        {
            // newBestFeas is the improving point.
            newBest = newBestFeas;
            // Workaround: If we do not have the point from which newBest was computed,
            // use refBestFeas instead.
            if (nullptr == newBest->getPointFrom() && nullptr != refBestFeas)
            {
                newBest->setPointFrom(refBestFeas, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
            }
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
            if (success2 > success)
            {
                success = success2;
            }
            if (success >= NOMAD::SuccessType::PARTIAL_SUCCESS)
            {
                // newBestInf is the improving point.
                newBest = newBestInf;
                // Workaround: If we do not have the point from which newBest was computed,
                // use refBestInf instead.
                if (nullptr == newBest->getPointFrom() && nullptr != refBestInf)
                {
                    newBest->setPointFrom(refBestInf, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));
                }
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
        if (success == NOMAD::SuccessType::UNSUCCESSFUL)
        {
            OUTPUT_DEBUG_START
            s = "Update: no success found";
            AddOutputDebug(s);
            OUTPUT_DEBUG_END
        }


        // Debug verification
        // Compare computed success with value from MegaIteration.
        // This is the value from the previous MegaIteration. If it
        // was not evaluated, ignore the test.
        // If queue is not cleared between runs, also ignore the test.
        bool clearEvalQueue = true;
        if (nullptr != evc)
        {
            clearEvalQueue = evc->getEvaluatorControlGlobalParams()->getAttributeValue<bool>("EVAL_QUEUE_CLEAR");
        }
        const bool megaIterEvaluated = (NOMAD::SuccessType::UNDEFINED != megaIter->getSuccessType());
        if (!clearEvalQueue && megaIterEvaluated && (success != megaIter->getSuccessType()))
        {
            s = "Warning: MegaIteration success type: ";
            s += NOMAD::enumStr(megaIter->getSuccessType());
            s += ". Is different than computed success type: " + NOMAD::enumStr(success);
            if (refBestFeas)
            {
                s += "\nRef best feasible:   " + refBestFeas->displayAll(computeTypeS);
            }
            if (newBestFeas)
            {
                s += "\nNew best feasible:   " + newBestFeas->displayAll(computeTypeS);
            }
            if (refBestInf)
            {
                s += "\nRef best infeasible: " + refBestInf->displayAll(computeTypeS);
            }
            if (newBestInf)
            {
                s += "\nNew best infeasible: " + newBestInf->displayAll(computeTypeS);
            }
            AddOutputWarning(s);
        }

        if (NOMAD::EvalType::BB == evalType)
        {
            // The directions of last successes may be used to sort points. Update values.
            // These dimensions are always in full space.
            
            
            auto dirFeas = (newBestFeas) ? newBestFeas->getDirection() : nullptr; //
            auto dirInf  = (newBestInf)  ? newBestInf->getDirection() : nullptr; //

            if (nullptr != dirFeas)
            {
                OUTPUT_INFO_START
                std::string dirStr = "New direction (feasible) ";
                dirStr += dirFeas->display();
                AddOutputInfo(dirStr);
                OUTPUT_INFO_END
            }
            if (nullptr != dirInf)
            {
                OUTPUT_INFO_START
                std::string dirStr = "New direction (infeasible) ";
                dirStr += dirInf->display();
                AddOutputInfo(dirStr);
                OUTPUT_INFO_END
            }

            if (nullptr != evc)
            {
                evc->setLastSuccessfulFeasDir(dirFeas);
                evc->setLastSuccessfulInfDir(dirInf);
            }
        }

        if (success >= NOMAD::SuccessType::PARTIAL_SUCCESS)
        {
            OUTPUT_INFO_START
            AddOutputInfo("Last Iteration Improving. Delta remains the same.");
            OUTPUT_INFO_END

        }
        else
        {
            OUTPUT_INFO_START
            AddOutputInfo("Last Iteration Unsuccessful. Delta is refined.");
            OUTPUT_INFO_END
            mesh->refineDeltaFrameSize();
        }
    }

    mesh->updatedeltaMeshSize();
    OUTPUT_INFO_START
    AddOutputInfo("delta mesh  size = " + mesh->getdeltaMeshSize().display());
    AddOutputInfo("Delta frame size = " + mesh->getDeltaFrameSize().display()); 
    OUTPUT_INFO_END


    return true;
}





