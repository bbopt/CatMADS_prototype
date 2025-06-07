
#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/LatinHypercubeSampling/LH.hpp"
#include "../../Algos/Mads/GMesh.hpp"
#include "../../Math/LHS.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::LH::init()
{
    setStepType(NOMAD::StepType::ALGORITHM_LH);
    verifyParentNotNull();

}

NOMAD::ArrayOfPoint NOMAD::LH::suggest ()
{
    generateTrialPoints();
    
    NOMAD::ArrayOfPoint xs;
    for (const auto& trialPoint : _trialPoints)
    {
        if (std::find(xs.begin(),xs.end(), *trialPoint.getX()) == xs.end())
        {
            xs.push_back(*trialPoint.getX());
        }
    }
    return xs;
}


void NOMAD::LH::startImp()
{
    // Default algorithm start
    // See issue #639
    NOMAD::Algorithm::startImp();
    
    generateTrialPoints();
}


void NOMAD::LH::generateTrialPointsImp()
{

    auto lhEvals = _runParams->getAttributeValue<size_t>("LH_EVAL");
    if (NOMAD::INF_SIZE_T == lhEvals)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "The number of evaluations for LH cannot be infinite.");
    }

    auto n = _pbParams->getAttributeValue<size_t>("DIMENSION");
    auto lowerBound = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND");

    if (!lowerBound.isComplete())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,getName() + " requires a complete lower bound vector");
    }

    auto upperBound = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND");
    if (!upperBound.isComplete())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,getName() + " requires a complete upper bound vector");
    }

    // Apply Latin Hypercube algorithm
    NOMAD::LHS lhs(n, lhEvals, lowerBound, upperBound);
    auto pointVector = lhs.Sample();


    // Create a mesh and project points on this mesh. It will ensure that parameters
    // BB_INPUT_TYPE and GRANULARITY are satisfied.
    auto mesh = std::make_shared<NOMAD::GMesh>(_pbParams, _runParams);
    mesh->setEnforceSanityChecks(false);
    // Modify mesh so it is the finest possible.
    // Note: GRANULARITY is already adjusted in regard to BB_INPUT_TYPE.
    NOMAD::ArrayOfDouble newMeshSize = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("GRANULARITY");
    auto eps = NOMAD::Double::getEpsilon();
    for (size_t i = 0; i < newMeshSize.size(); i++)
    {
        if (0 == newMeshSize[i])
        {
            // No granularity: Set mesh size to Epsilon.
            newMeshSize[i] = eps;
        }
    }

    mesh->setDeltas(newMeshSize, newMeshSize);
    auto center = NOMAD::Point(n, 0);

    for (auto point : pointVector)
    {
        // First, project on mesh.
        if (_runParams->getAttributeValue<bool>("SEARCH_METHOD_MESH_PROJECTION"))
        {
            point = mesh->projectOnMesh(point, center);
        }
        // Second, snap to bounds.
        point.snapToBounds(lowerBound, upperBound);

        NOMAD::EvalPoint evalPoint(point);
        // Test if the point is inserted correctly
        bool inserted = insertTrialPoint(evalPoint);
        OUTPUT_INFO_START
        std::string s = "Generated point";
        s += (inserted) ? ": " : " not inserted: ";
        s += evalPoint.display();
        AddOutputInfo(s);
        OUTPUT_INFO_END
    }
}


bool NOMAD::LH::runImp()
{
    bool foundBetter = false;

    if ( ! _stopReasons->checkTerminate() )
    {
        foundBetter = evalTrialPoints(this);
    }
    auto LHStopReasons = NOMAD::AlgoStopReasons<NOMAD::LHStopType>::get( _stopReasons );
    if (NOMAD::EvcInterface::getEvaluatorControl()->testIf(NOMAD::EvalMainThreadStopType::ALL_POINTS_EVALUATED))
    {
        LHStopReasons->set( NOMAD::LHStopType::ALL_POINTS_EVALUATED );
    }
    
    // Update local stats with evaluations done
    // This is normally done in postProcessing but LH does not call this function
    size_t nbTrialPointsEvaluated = 0;
    auto evc = NOMAD::EvcInterface::getEvaluatorControl();
    auto evalType = NOMAD::EvalType::BB;
    if (nullptr != evc)
    {
        evalType = evc->getCurrentEvalType();
    }
    for (const auto& trialPoint : _trialPoints)
    {
        // We are looking for trial points that have been evaluated
        if (trialPoint.isEvalOk(evalType))
        {
            nbTrialPointsEvaluated ++;
        }
    }
    NOMAD::IterationUtils::_trialPointStats.incrementEvalsDone(nbTrialPointsEvaluated, evalType);

    return foundBetter;
}

