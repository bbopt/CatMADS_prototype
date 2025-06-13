#include "../Util/StopReason.hpp"

//// Dictionary funct for BaseStopType
template<> DLL_UTIL_API std::map<NOMAD::BaseStopType,std::string> & NOMAD::StopReason<NOMAD::BaseStopType>::dict() const
{
    static std::map<NOMAD::BaseStopType,std::string> dictionary= {
        {NOMAD::BaseStopType::STARTED,"Started"},   // Set at the beginning of a Step
        {NOMAD::BaseStopType::MAX_TIME_REACHED,"Maximum allowed time reached"},
        {NOMAD::BaseStopType::INITIALIZATION_FAILED,"Initialization failure"},
        {NOMAD::BaseStopType::ERROR,"Error"},
        {NOMAD::BaseStopType::UNKNOWN_STOP_REASON,"Unknown"},
        {NOMAD::BaseStopType::CTRL_C,"Ctrl-C"},
        {NOMAD::BaseStopType::HOT_RESTART,"Hot restart interruption"},
        {NOMAD::BaseStopType::USER_GLOBAL_STOP,"Global user stop in a callback function"}

    };
    return dictionary;
}

// Returns true only to terminate an algorithm (Mads, ...)
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::BaseStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::BaseStopType::MAX_TIME_REACHED:
        case NOMAD::BaseStopType::INITIALIZATION_FAILED:
        case NOMAD::BaseStopType::ERROR:
        case NOMAD::BaseStopType::UNKNOWN_STOP_REASON:
        case NOMAD::BaseStopType::CTRL_C:
        case NOMAD::BaseStopType::USER_GLOBAL_STOP:
            return true;
        case NOMAD::BaseStopType::STARTED:
        case NOMAD::BaseStopType::HOT_RESTART:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for algo terminate");
    }
    return false;
}

// Dictionary function for MadsStopType
template<> DLL_UTIL_API std::map<NOMAD::MadsStopType,std::string> & NOMAD::StopReason<NOMAD::MadsStopType>::dict() const
{
    static std::map<NOMAD::MadsStopType,std::string> dictionary = {
        {NOMAD::MadsStopType::STARTED,"Started"},   // Set at the beginning of a Step
        {NOMAD::MadsStopType::MAX_MESH_INDEX_REACHED,"Max mesh index reached"},
        {NOMAD::MadsStopType::MIN_MESH_INDEX_REACHED,"Min mesh index reached"},
        {NOMAD::MadsStopType::MESH_PREC_REACHED,"Mesh minimum precision reached"},
        {NOMAD::MadsStopType::MIN_MESH_SIZE_REACHED,"Min mesh size reached"},
        {NOMAD::MadsStopType::MIN_FRAME_SIZE_REACHED,"Min frame size reached"},
        {NOMAD::MadsStopType::PONE_SEARCH_FAILED,"Phase one search did not return a feasible point"},
        {NOMAD::MadsStopType::UPDATE_FAILED,"Update failed"},
        {NOMAD::MadsStopType::X0_FAIL,"Problem with starting point evaluation"}
    };
    return dictionary;
}


// Returns true only to terminate an algorithm (Mads, ...)
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::MadsStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::MadsStopType::MESH_PREC_REACHED:
        case NOMAD::MadsStopType::MAX_MESH_INDEX_REACHED:
        case NOMAD::MadsStopType::MIN_MESH_INDEX_REACHED:
        case NOMAD::MadsStopType::MIN_MESH_SIZE_REACHED:
        case NOMAD::MadsStopType::MIN_FRAME_SIZE_REACHED:
        case NOMAD::MadsStopType::PONE_SEARCH_FAILED:
        case NOMAD::MadsStopType::UPDATE_FAILED:
        case NOMAD::MadsStopType::X0_FAIL:
            return true;
        case NOMAD::MadsStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for algo terminate");
    }
    return false;
}

// Dictionary function for PhaseOneStopType
template<> DLL_UTIL_API std::map<NOMAD::PhaseOneStopType,std::string> & NOMAD::StopReason<NOMAD::PhaseOneStopType>::dict() const
{
    static std::map<NOMAD::PhaseOneStopType,std::string> dictionary = {
        {NOMAD::PhaseOneStopType::STARTED,"Started"},   // Set at the beginning of a Step
        {NOMAD::PhaseOneStopType::NO_FEAS_PT,"No feasible point obtained by PhaseOne search"},
        {NOMAD::PhaseOneStopType::MADS_FAIL,"Mads has terminated and no feasible point obtained"}
    };
    return dictionary;
}


// Dictionary function for SSDMadsStopType
template<> DLL_UTIL_API std::map<NOMAD::SSDMadsStopType,std::string> & NOMAD::StopReason<NOMAD::SSDMadsStopType>::dict() const
{
    static std::map<NOMAD::SSDMadsStopType,std::string> dictionary = {
        {NOMAD::SSDMadsStopType::STARTED,"Started"},   // Set at the beginning of a Step
        {NOMAD::SSDMadsStopType::X0_FAIL,"Problem with starting point evaluation"}
        //{NOMAD::SSDMadsStopType::MADS_FAIL,"Mads has terminated but no feasible point obtained"}
    };
    return dictionary;
}


// Returns true only to terminate an algorithm using PhaseOne (Mads, ...)
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::PhaseOneStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::PhaseOneStopType::NO_FEAS_PT:
        case NOMAD::PhaseOneStopType::MADS_FAIL:
            return true;
        case NOMAD::PhaseOneStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for terminate");
    }
    return false;
}


template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::SSDMadsStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::SSDMadsStopType::X0_FAIL:
        //case NOMAD::SSDMadsStopType::MADS_FAIL:
            return true;
        case NOMAD::SSDMadsStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for terminate");
    }
    return false;
}


// Dictionary function for LHStopType
template<> DLL_UTIL_API std::map<NOMAD::LHStopType,std::string> & NOMAD::StopReason<NOMAD::LHStopType>::dict() const
{
    static std::map<NOMAD::LHStopType,std::string> dictionary = {
        {NOMAD::LHStopType::STARTED,"Started"},   // Set at the beginning of an EvaluatorControl Run
        {NOMAD::LHStopType::NO_POINTS_GENERATED, "No points generated by Latin Hypercube"},
        {NOMAD::LHStopType::ALL_POINTS_EVALUATED,"No more points to evaluate"}
    };
    return dictionary;
}

// Dictionary function for CSStopType
template<> DLL_UTIL_API std::map<NOMAD::CSStopType,std::string> & NOMAD::StopReason<NOMAD::CSStopType>::dict() const
{
    static std::map<NOMAD::CSStopType,std::string> dictionary = {
        {NOMAD::CSStopType::STARTED,"Started"},   // Set at the beginning of a Step
        {NOMAD::CSStopType::MESH_PREC_REACHED,"Mesh minimum precision reached"},
        {NOMAD::CSStopType::GRANULARITY_REACHED," minimum Granularity value reached"},
        {NOMAD::CSStopType::MIN_MESH_SIZE_REACHED,"Min mesh size reached"},
        {NOMAD::CSStopType::MIN_FRAME_SIZE_REACHED,"Min frame size reached"},
        {NOMAD::CSStopType::X0_FAIL,"Problem with starting point evaluation"}
    };
    return dictionary;
}

// Dictionary function for VNSStopType
template<> DLL_UTIL_API std::map<NOMAD::VNSStopType,std::string> & NOMAD::StopReason<NOMAD::VNSStopType>::dict() const
{
    static std::map<NOMAD::VNSStopType,std::string> dictionary = {
        {NOMAD::VNSStopType::STARTED,"Started"},   // Set at the beginning of a Step
        {NOMAD::VNSStopType::X0_FAILED,"Pb with starting point evaluation"},
        {NOMAD::VNSStopType::INITIAL_FAILED,"Pb during initialization"},
        {NOMAD::VNSStopType::SUBPB_MADS_FAILED,"Subproblem mads failed"},
        {NOMAD::VNSStopType::SHAKING_FAILED,"Shaking failed to generated starting points"},
        {NOMAD::VNSStopType::SINGLE_PASS_COMPLETED,"A single mads mega search poll completed."}
    };
    return dictionary;
}


// Returns true when Latin Hypercube Sampling is complete, or no points generated
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::LHStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::LHStopType::ALL_POINTS_EVALUATED:
        case NOMAD::LHStopType::NO_POINTS_GENERATED:
            return true;
        case NOMAD::LHStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for algo terminate");
    }
    return false;
}

// Returns true when Coordinate Search  is complete, or no points generated
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::CSStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::CSStopType::MESH_PREC_REACHED:
        case NOMAD::CSStopType::MIN_MESH_SIZE_REACHED:
        case NOMAD::CSStopType::MIN_FRAME_SIZE_REACHED:
        case NOMAD::CSStopType::GRANULARITY_REACHED:
        case NOMAD::CSStopType::X0_FAIL:
            return true;
        case NOMAD::CSStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for algo terminate");
    }
    return false;
}

// Dictionary function for NMStopType
template<> DLL_UTIL_API std::map<NOMAD::NMStopType,std::string> & NOMAD::StopReason<NOMAD::NMStopType>::dict() const
{
    static std::map<NOMAD::NMStopType,std::string> dictionary = {
        {NOMAD::NMStopType::STARTED,"Started"},   // Set at the beginning of an EvaluatorControl Run
        {NOMAD::NMStopType::TOO_SMALL_SIMPLEX, "Simplex Y is too small"}, ///< Not used
        {NOMAD::NMStopType::SIMPLEX_RANK_INSUFFICIENT, "Rank of the matrix DZ is too small"}, ///< Not used
        {NOMAD::NMStopType::INITIAL_FAILED,"Initialization has failed"},
        {NOMAD::NMStopType::REFLECT_FAILED,"Reflect step has failed"}              ,
        {NOMAD::NMStopType::EXPANSION_FAILED,"Expansion step has failed"}            ,
        {NOMAD::NMStopType::OUTSIDE_CONTRACTION_FAILED,"Outside contraction step has failed"}  ,
        {NOMAD::NMStopType::INSIDE_CONTRACTION_FAILED,"Inside contraction step failed"}   ,
        {NOMAD::NMStopType::SHRINK_FAILED,"Shrink step has failed"}               ,
        {NOMAD::NMStopType::UNDEFINED_STEP,"Unknown step"}              ,
        {NOMAD::NMStopType::INSERTION_FAILED,"Insertion of points has failed"}            ,
        {NOMAD::NMStopType::X0_FAILED,"No X0 provided or cannot evaluate X0"},
        {NOMAD::NMStopType::NM_SINGLE_COMPLETED,"NM with a single iteration is completed"},
        {NOMAD::NMStopType::NM_STOP_ON_SUCCESS,"NM iterations stopped on eval success"},
        {NOMAD::NMStopType::NM_STOP_NO_SHRINK,"NM iterations stopped without shrink"}
    };
    return dictionary;
}


// Returns true only when Nelder Mead algorithm is complete
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::NMStopType>::checkTerminate() const
{

    switch ( _stopReason )
    {
        case NOMAD::NMStopType::INITIAL_FAILED:
        case NOMAD::NMStopType::X0_FAILED:
        case NOMAD::NMStopType::REFLECT_FAILED:
        case NOMAD::NMStopType::EXPANSION_FAILED:
        case NOMAD::NMStopType::INSIDE_CONTRACTION_FAILED:
        case NOMAD::NMStopType::OUTSIDE_CONTRACTION_FAILED:
        case NOMAD::NMStopType::SHRINK_FAILED:
        case NOMAD::NMStopType::INSERTION_FAILED:
        case NOMAD::NMStopType::NM_SINGLE_COMPLETED:
        case NOMAD::NMStopType::NM_STOP_ON_SUCCESS:
        case NOMAD::NMStopType::NM_STOP_NO_SHRINK:
        case NOMAD::NMStopType::UNDEFINED_STEP:
            return true;
        case NOMAD::NMStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All NM stop types must be checked for algo terminate");
    }
    return false;
}

// Dictionary function for RandomAlgoStopType
template<> DLL_UTIL_API std::map<NOMAD::RandomAlgoStopType,std::string> & NOMAD::StopReason<NOMAD::RandomAlgoStopType>::dict() const
{
    static std::map<NOMAD::RandomAlgoStopType,std::string> dictionary = {
        {NOMAD::RandomAlgoStopType::STARTED,"Started"},   // Set at the beginning of an EvaluatorControl Run
        {NOMAD::RandomAlgoStopType::UNDEFINED_STEP,"Unknown step"},
        {NOMAD::RandomAlgoStopType::INITIAL_FAILED,"Algo initialization failed"},
        {NOMAD::RandomAlgoStopType::UPDATE_FAILED,"Algo update of best point failed"},
        {NOMAD::RandomAlgoStopType::X0_FAILED,"No X0 provided or cannot evaluate X0"},
        {NOMAD::RandomAlgoStopType::ALL_POINTS_EVALUATED,"All trial points evaluated, budget spent"},
        {NOMAD::RandomAlgoStopType::SINGLE_PASS_COMPLETED,"A single iteration is completed"}
    };
    return dictionary;
}

// Returns true only when template algorithm (random) is complete
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::RandomAlgoStopType>::checkTerminate() const
{

    switch ( _stopReason )
    {
        case NOMAD::RandomAlgoStopType::ALL_POINTS_EVALUATED:
        case NOMAD::RandomAlgoStopType::X0_FAILED:
        case NOMAD::RandomAlgoStopType::INITIAL_FAILED:
        case NOMAD::RandomAlgoStopType::UPDATE_FAILED:
        case NOMAD::RandomAlgoStopType::UNDEFINED_STEP:
        case NOMAD::RandomAlgoStopType::SINGLE_PASS_COMPLETED:
            return true;
        case NOMAD::RandomAlgoStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All template algo stop types must be checked for algo terminate");
    }
    return false;
}

// Dictionary function for SimpleLineSearchStopType
template<> DLL_UTIL_API std::map<NOMAD::SimpleLineSearchStopType,std::string> & NOMAD::StopReason<NOMAD::SimpleLineSearchStopType>::dict() const
{
    static std::map<NOMAD::SimpleLineSearchStopType,std::string> dictionary = {
        {NOMAD::SimpleLineSearchStopType::STARTED,"Started"},   // Set at the beginning of an EvaluatorControl Run
        {NOMAD::SimpleLineSearchStopType::SPECULATIVE_SUCCESSFUL,"Speculative is successful, no line search"},
        {NOMAD::SimpleLineSearchStopType::ALL_POINTS_EVALUATED,"All trial points evaluated, budget spent"}
    };
    return dictionary;
}

// Returns true only when template algorithm (random) is complete
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::SimpleLineSearchStopType>::checkTerminate() const
{

    switch ( _stopReason )
    {
        case NOMAD::SimpleLineSearchStopType::ALL_POINTS_EVALUATED:
        case NOMAD::SimpleLineSearchStopType::SPECULATIVE_SUCCESSFUL:
            return true;
        case NOMAD::SimpleLineSearchStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All simple line search stop types must be checked for algo terminate");
    }
    return false;
}


// Returns true only when VNS Mads algorithm is complete
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::VNSStopType>::checkTerminate() const
{

    switch ( _stopReason )
    {
        case NOMAD::VNSStopType::INITIAL_FAILED:
        case NOMAD::VNSStopType::X0_FAILED:
        case NOMAD::VNSStopType::SUBPB_MADS_FAILED:
        case NOMAD::VNSStopType::SHAKING_FAILED:
        case NOMAD::VNSStopType::SINGLE_PASS_COMPLETED:
            return true;
        case NOMAD::VNSStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All VNS stop types must be checked for algo terminate");
    }
    return false;
}


// Dictionary function for IterStopType
template<> DLL_UTIL_API std::map<NOMAD::IterStopType,std::string> & NOMAD::StopReason<NOMAD::IterStopType>::dict() const
{
    static std::map<NOMAD::IterStopType,std::string> dictionary = {
        {NOMAD::IterStopType::STARTED,"Started"},   // Set at the beginning of a Step task
        {NOMAD::IterStopType::MAX_ITER_REACHED,"Maximum number of iterations reached"},
        {NOMAD::IterStopType::STOP_ON_FEAS,"A feasible point is reached"},
        {NOMAD::IterStopType::PHASE_ONE_COMPLETED,"PhaseOne completed"},
        {NOMAD::IterStopType::USER_ITER_STOP,"Local (iter) user stop"},
        {NOMAD::IterStopType::USER_ALGO_STOP,"Local (algo) user stop"}
    };
    return dictionary;
}


// Returns true only to terminate an algorithm (Mads, ...)
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::IterStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::IterStopType::MAX_ITER_REACHED:
        case NOMAD::IterStopType::STOP_ON_FEAS:
        case NOMAD::IterStopType::PHASE_ONE_COMPLETED:
        case NOMAD::IterStopType::USER_ITER_STOP:
        case NOMAD::IterStopType::USER_ALGO_STOP:
            return true;
        case NOMAD::IterStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for algo terminate");

    }
    return false;
}


// Dictionary function for EvalGlobalStopType
template<> DLL_UTIL_API std::map<NOMAD::EvalGlobalStopType,std::string> & NOMAD::StopReason<NOMAD::EvalGlobalStopType>::dict() const
{
    static std::map<NOMAD::EvalGlobalStopType,std::string> dictionary = {
        {NOMAD::EvalGlobalStopType::STARTED,                  "Started"},   // Set at the beginning of an EvaluatorControl Run
        {NOMAD::EvalGlobalStopType::MAX_BB_EVAL_REACHED,      "Maximum number of blackbox evaluations"},
        {NOMAD::EvalGlobalStopType::MAX_SURROGATE_EVAL_OPTIMIZATION_REACHED, "Maximum number of surrogate evaluations"},
        {NOMAD::EvalGlobalStopType::MAX_EVAL_REACHED,         "Maximum number of total evaluations"},
        {NOMAD::EvalGlobalStopType::MAX_BLOCK_EVAL_REACHED,   "Maximum number of block eval reached"},
        {NOMAD::EvalGlobalStopType::CUSTOM_GLOBAL_STOP,   "User requested global stop after an evaluation"}
    };
    return dictionary;
}


// Dictionary function for EvalMainThreadStopType
template<> DLL_UTIL_API std::map<NOMAD::EvalMainThreadStopType,std::string> & NOMAD::StopReason<NOMAD::EvalMainThreadStopType>::dict() const
{
    static std::map<NOMAD::EvalMainThreadStopType,std::string> dictionary = {
        {NOMAD::EvalMainThreadStopType::STARTED,                  "Started"},   // Set at the beginning of an EvaluatorControl Run
        {NOMAD::EvalMainThreadStopType::LAP_MAX_BB_EVAL_REACHED,  "Maximum number of blackbox evaluations for a sub algorithm run (lap run)"},
        {NOMAD::EvalMainThreadStopType::SUBPROBLEM_MAX_BB_EVAL_REACHED,  "Maximum number of blackbox evaluations for a subproblem run"},
        {NOMAD::EvalMainThreadStopType::OPPORTUNISTIC_SUCCESS,    "Success found and opportunistic strategy maybe used (if set)"},
        {NOMAD::EvalMainThreadStopType::CUSTOM_OPPORTUNISTIC_ITER_STOP,    "Custom opportunistic iteration stop detected via post-evaluation callback"},
        {NOMAD::EvalMainThreadStopType::CUSTOM_OPPORTUNISTIC_EVAL_STOP,    "Custom opportunistic evaluation stop detected via post-evaluation callback"},
        {NOMAD::EvalMainThreadStopType::EMPTY_LIST_OF_POINTS,     "Tried to eval an empty list"},
        {NOMAD::EvalMainThreadStopType::ALL_POINTS_EVALUATED,     "No more points to evaluate"},
        {NOMAD::EvalMainThreadStopType::MAX_MODEL_EVAL_REACHED,   "Maximum number of model evaluations reached"}
    };
    return dictionary;
}

template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::EvalGlobalStopType>::checkTerminate() const
{
    // Returns true only to terminate an algorithm (Mads, ...)
    switch ( _stopReason )
    {
        case NOMAD::EvalGlobalStopType::MAX_BB_EVAL_REACHED:
        case NOMAD::EvalGlobalStopType::MAX_SURROGATE_EVAL_OPTIMIZATION_REACHED:
        case NOMAD::EvalGlobalStopType::MAX_EVAL_REACHED:
        case NOMAD::EvalGlobalStopType::MAX_BLOCK_EVAL_REACHED:
        case NOMAD::EvalGlobalStopType::CUSTOM_GLOBAL_STOP:
            return true;
        case NOMAD::EvalGlobalStopType::STARTED:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for algo terminate");
    }
    return false;
}


template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::EvalMainThreadStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::EvalMainThreadStopType::LAP_MAX_BB_EVAL_REACHED:
        case NOMAD::EvalMainThreadStopType::SUBPROBLEM_MAX_BB_EVAL_REACHED:
        case NOMAD::EvalMainThreadStopType::MAX_MODEL_EVAL_REACHED:
        case NOMAD::EvalMainThreadStopType::CUSTOM_OPPORTUNISTIC_ITER_STOP:
            return true;
        case NOMAD::EvalMainThreadStopType::STARTED:
        case NOMAD::EvalMainThreadStopType::OPPORTUNISTIC_SUCCESS:
        case NOMAD::EvalMainThreadStopType::EMPTY_LIST_OF_POINTS:
        case NOMAD::EvalMainThreadStopType::ALL_POINTS_EVALUATED:
        case NOMAD::EvalMainThreadStopType::CUSTOM_OPPORTUNISTIC_EVAL_STOP:
            return false;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for algo terminate");
    }
    return false;
}



// Dictionary function for ModelStopType
template<> DLL_UTIL_API std::map<NOMAD::ModelStopType,std::string> & NOMAD::StopReason<NOMAD::ModelStopType>::dict() const
{
    static std::map<NOMAD::ModelStopType,std::string> dictionary = {
        {NOMAD::ModelStopType::STARTED, "Started"},   // Set at the beginning of a Step
        {NOMAD::ModelStopType::ORACLE_FAIL, "Oracle failed generating points"},

        {NOMAD::ModelStopType::MODEL_OPTIMIZATION_FAIL, "Model Optimization has failed"},
        {NOMAD::ModelStopType::INITIAL_FAIL, "Cannot initialize model"},
        {NOMAD::ModelStopType::NOT_ENOUGH_POINTS, "Not enough points to build model"},
        {NOMAD::ModelStopType::NO_NEW_POINTS_FOUND, "Models optimization did not find new points"},
        {NOMAD::ModelStopType::EVAL_FAIL, "Problem with Model evaluation"},
        {NOMAD::ModelStopType::X0_FAIL, "Problem with starting point evaluation"},
        {NOMAD::ModelStopType::ALL_POINTS_EVALUATED,"No more points to evaluate"},
        {NOMAD::ModelStopType::MODEL_SINGLE_PASS_COMPLETED,"A single pass to create trial point has been completed successfully."}
    };
    return dictionary;
}


// Returns true only to terminate a model based algorithms (sgtelib, quad, ...)
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::ModelStopType>::checkTerminate() const
{
    switch ( _stopReason )
    {
        case NOMAD::ModelStopType::STARTED:
        case NOMAD::ModelStopType::ALL_POINTS_EVALUATED:
        case NOMAD::ModelStopType::MODEL_SINGLE_PASS_COMPLETED:
            return false;
        case NOMAD::ModelStopType::MODEL_OPTIMIZATION_FAIL:
        case NOMAD::ModelStopType::ORACLE_FAIL:
        case NOMAD::ModelStopType::INITIAL_FAIL:
        case NOMAD::ModelStopType::NOT_ENOUGH_POINTS:
        case NOMAD::ModelStopType::NO_NEW_POINTS_FOUND:
        case NOMAD::ModelStopType::EVAL_FAIL:
        case NOMAD::ModelStopType::X0_FAIL:
            return true;
        default:
            throw NOMAD::Exception ( __FILE__, __LINE__,"All stop types must be checked for algo terminate");
    }
    return false;
}


// Dictionary function for ModelStopType
template<> DLL_UTIL_API std::map<NOMAD::QPStopType,std::string> & NOMAD::StopReason<NOMAD::QPStopType>::dict() const
{
    static std::map<NOMAD::QPStopType,std::string> dictionary = {
        {NOMAD::QPStopType::STARTED, "Started"},   // Set at the beginning of a Step
        {NOMAD::QPStopType::CONVERGED, "Optimization converged"},
        {NOMAD::QPStopType::FAILED, "Optimization has failed"},
        {NOMAD::QPStopType::STAGNATION_ITERATES, "Optimization stagnation in iterates"},
        {NOMAD::QPStopType::MAX_ITERATIONS, "Optimization reached max iteration limit"}
    };
    return dictionary;
}


// Not used.
template<> DLL_UTIL_API bool NOMAD::StopReason<NOMAD::QPStopType>::checkTerminate() const
{
    return false;
}
