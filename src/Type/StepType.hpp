/**
 \file   StepType.hpp
 \brief  Types for Steps that generate points: Search and Poll methods, and Algorithms
 \author Viviane Rochon Montplaisir
 \date   June 2021
 \see    StepType.cpp
 */

#ifndef __NOMAD_4_5_STEP_TYPE__
#define __NOMAD_4_5_STEP_TYPE__

#include <map>
#include <sstream>
#include <vector>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

enum class StepType
{
    ALGORITHM_LH,               ///< Algorithm Latin Hypercube
    ALGORITHM_CS,               ///< Algorithm Coordinate Search
    ALGORITHM_DMULTIMADS,       ///< Algorithm DmultiMads
    ALGORITHM_MADS,             ///< Algorithm Mads
    ALGORITHM_SIMPLE_MADS,      ///< Algorithm Mads trimmed 
    ALGORITHM_NM,               ///< Algorithm Nelder-Mead
    ALGORITHM_PHASE_ONE,        ///< Phase One
    ALGORITHM_PSD_MADS_SUBPROBLEM, ///< Subproblem in PSD-Mads
    ALGORITHM_PSD_MADS,         ///< Algorithm PSD-Mads
    ALGORITHM_QPSOLVER,         ///< Algorithm QPSolver
    ALGORITHM_QUAD_MODEL,       ///< Algorithm Quad Model
    ALGORITHM_SGTELIB_MODEL,    ///< Algorithm Quad Model
    ALGORITHM_SSD_MADS,         ///< Algorithm SSD-Mads
    ALGORITHM_RANDOM,           ///< Algorithm random (to be used as a template example for a new algorithm)
    ALGORITHM_VNS_MADS,         ///< Algorithm VNS-Mads
    ALGORITHM_DISCO_MADS,       ///< Algorithm DISCO-Mads
    ALGORITHM_COOP_MADS,        ///< Algorithm DISCO-Mads
    INITIALIZATION,             ///< Initialization step
    ITERATION,                  ///< Iteration step
    MAIN,                       ///< Main step
    MAIN_OBSERVE,               ///< Main step for Observe
    MAIN_SUGGEST,               ///< Main step for Suggest
    MEGA_ITERATION,             ///< MegaIteration step
    MEGA_SEARCH_POLL,           ///< MegaSearchPoll

    NM_CONTINUE,                ///< NM continue
    NM_EXPAND,                  ///< NM Expansion
    NM_INITIAL,                 ///< NM initial step type
    NM_INITIALIZE_SIMPLEX,      ///< NM initialize simplex
    NM_INSERT_IN_Y,             ///< NM insert in Y
    NM_INSIDE_CONTRACTION,      ///< NM Inside Contraction
    NM_ITERATION,               ///< NM Iteration
    NM_OUTSIDE_CONTRACTION,     ///< NM Outside Contraction
    NM_REFLECT,                 ///< NM Reflect
    NM_SHRINK,                  ///< NM Shrink
    NM_UNSET,                   ///< NM step type not set

    MODEL_OPTIMIZE,                   ///<  model sub-optimization
    POLL,                       ///< Mads Poll
    SIMPLE_POLL,                ///< Simple Mads Poll
    CS_POLL,                    ///< Coordinate Search poll  
    REVEALING_POLL,             ///< Revealing poll      
    POLL_METHOD_DOUBLE,         ///< Double poll method
    POLL_METHOD_ORTHO_NPLUS1_NEG, ///< Ortho N+1 neg poll method
    POLL_METHOD_ORTHO_NPLUS1_QUAD, ///< Ortho N+1 quad poll method
    POLL_METHOD_ORTHO_2N,       ///< Ortho 2N poll method
    POLL_METHOD_QR_2N,           ///< QR 2N poll method
    POLL_METHOD_SINGLE,         ///< Single poll method
    POLL_METHOD_UNI_NPLUS1,     ///< Uniform N+1 poll method
    POLL_METHOD_USER,           ///< User custom poll method
    CS_POLL_METHOD,             ///< Coordinate Search poll method
    EXTENDED_POLL,              ///< Mads Extended Poll
    EXTENDED_POLL_METHOD_ALGO,  ///<  Extended poll method algo (Custom from user)
    SEARCH,                     ///< Search
    SEARCH_METHOD_ALGO_RANDOM,///< Template for a search method using an algo (iterations)
    SEARCH_METHOD_CACHE,           ///< Cache search method to sync. parallel algos
    SEARCH_METHOD_DMULTIMADS_MIDDLEPOINT, ///< DMultiMads Middle Point search method
    SEARCH_METHOD_DMULTIMADS_EXPANSIONINT_LINESEARCH, ///< DMultiMads Expansion integer line search method
    SEARCH_METHOD_DMULTIMADS_QUAD_DMS, ///< DMultiMads Quad DMS search method
    SEARCH_METHOD_LH,           ///< Latin hypercube search method
    SEARCH_METHOD_NM,           ///< Nelder-Mead search method
    SEARCH_METHOD_QUAD_MODEL,   ///< Quadratic model search method
    SEARCH_METHOD_QUAD_MODEL_SLD,///< Quadratic model (SLD) search method
    SEARCH_METHOD_REVEALING,     ///< Revealing search method (revealing poll in DiscoMads)  
    SEARCH_METHOD_SGTELIB_MODEL,///< Sgtelib model search method
    SEARCH_METHOD_SIMPLE_RANDOM,///< Template for a simple (no iteration) search method
    SEARCH_METHOD_SIMPLE_LINE_SEARCH,  ///< Simple line search method
    SEARCH_METHOD_SPECULATIVE,  ///< Speculative search method
    SEARCH_METHOD_USER,         ///< User-defined search method
    SEARCH_METHOD_VNS_MADS,     ///< VNS Mads search method
    SEARCH_METHOD_VNSMART_MADS, ///< VNS Mads smart search method
    SURROGATE_EVALUATION,       ///< Evaluating trial points using static surrogate
    MODEL_EVALUATION,           ///< Evaluating trial points using dynamic model
    QUAD_MODEL_SORT,           ///< Build quad model to sort trial points
    TERMINATION,                ///< Termination
    UNDEFINED,                  ///< Unknown value (default)
    UPDATE                      ///< Update step
};


/// Definition for a vector of StepTypes
typedef std::vector<StepType> StepTypeList;

/// Helper to test if a StepType represents an Algorithm (ALGORITHM_MADS, etc).
DLL_UTIL_API bool isAlgorithm(const StepType& stepType);

DLL_UTIL_API std::map<StepType, std::string>& dictStepType();

// Convert an StepType to a string
DLL_UTIL_API std::string stepTypeToString(const StepType& stepType);

// Convert a StepTypeList to a string; show only pertinent information.
DLL_UTIL_API std::string StepTypeListToString(const StepTypeList& stepTypeList);


inline std::ostream& operator<<(std::ostream& out, const StepType &stepType)
{
    out << stepTypeToString(stepType);
    return out;
}


#include "../nomad_nsend.hpp"
#endif  // __NOMAD_4_5_STEP_TYPE__
