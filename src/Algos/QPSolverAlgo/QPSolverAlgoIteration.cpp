#include "../../Algos/QPSolverAlgo/QPSolverAlgoIteration.hpp"
#include "../../Algos/QPSolverAlgo/QPSolverOptimize.hpp"

bool NOMAD::QPSolverAlgoIteration::runImp()
{
    // Iteration cannot generate all points before evaluation
    verifyGenerateAllPointsBeforeEval(NOMAD_PRETTY_FUNCTION, false);

    bool iterationSuccess = false;
    
    // Model Update is handled in start().
    if (!_stopReasons->checkTerminate() && _model->is_ready() )
    {
        
        // Initialize optimize member on model
        NOMAD::QPSolverOptimize optimize (this, _pbParams, false /* do not perform on a scaled models */);
        
        
        // Optimize to find oracle points on this model
        optimize.start();
        iterationSuccess = optimize.run();
        optimize.end();
        
        

        // Update MegaIteration success type
        _success = optimize.getTrialPointsSuccessType();
        auto megaIter = getParentOfType<NOMAD::MegaIteration*>();
        megaIter->setSuccessType(_success);
        
    }

    // End of the iteration: iterationSuccess is true if we have a success.
    return iterationSuccess;


}
