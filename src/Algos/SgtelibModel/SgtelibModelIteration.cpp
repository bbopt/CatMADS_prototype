
#include "../../Algos/SgtelibModel/SgtelibModel.hpp"
#include "../../Algos/SgtelibModel/SgtelibModelIteration.hpp"
#include "../../Algos/SgtelibModel/SgtelibModelUpdate.hpp"

void NOMAD::SgtelibModelIteration::init()
{
    setStepType(NOMAD::StepType::ITERATION);

    // Initialize optimize member - model optimizer on sgte
    auto modelAlgo = getParentOfType<NOMAD::SgtelibModel*>();
    _optimize = std::make_shared<NOMAD::SgtelibModelOptimize>(modelAlgo,
                                                    _runParams, _pbParams);
}


void NOMAD::SgtelibModelIteration::startImp()
{
    // Model update
    // Use the cache to determine a sgtelib model
    // Update has a side effect of setting the _ready member.
    NOMAD::SgtelibModelUpdate update(this);
    update.start();
    update.run();
    update.end();
}


bool NOMAD::SgtelibModelIteration::runImp()
{
    //verifyGenerateAllPointsBeforeEval("Iteration::run()", false);

    bool optimizationOk = false;

    // Model Update is handled in start().

    auto modelAlgo = getParentOfType<NOMAD::SgtelibModel*>();
    if (!_stopReasons->checkTerminate() && modelAlgo->isReady())
    {
        // Use the optimizer to find oracle points on this model

        // Setup Pb parameters just before optimization.
        // This way, we get the best X0s.
        _optimize->setupPbParameters(modelAlgo->getExtendedLowerBound(),
                                     modelAlgo->getExtendedUpperBound());

        _optimize->start();
        optimizationOk = _optimize->run();
        _optimize->end();
    }

    // End of the iteration: return value of optimizationOk
    return optimizationOk;
}


// Oracle points are the best points found in sub optimization on sgtelib model.
const NOMAD::EvalPointSet& NOMAD::SgtelibModelIteration::getOraclePoints() const
{
    return _optimize->getOraclePoints();
}

