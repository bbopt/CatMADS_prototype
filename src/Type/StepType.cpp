/**
 \file   StepType.cpp
 \brief  types for Types for Steps that generate points (implementation)
 \author Viviane Rochon Montplaisir
 \date   June 2021
 \see    StepType.hpp
 */

#include "../Type/StepType.hpp"
#include "../Util/Exception.hpp"
#include "../Util/utils.hpp"


bool NOMAD::isAlgorithm(const StepType& stepType)
{
    switch (stepType)
    {
        case NOMAD::StepType::ALGORITHM_LH:
        case NOMAD::StepType::ALGORITHM_CS:
        case NOMAD::StepType::ALGORITHM_DMULTIMADS:
        case NOMAD::StepType::ALGORITHM_MADS:
        case NOMAD::StepType::ALGORITHM_SIMPLE_MADS:
        case NOMAD::StepType::ALGORITHM_NM:
        case NOMAD::StepType::ALGORITHM_PHASE_ONE:
        case NOMAD::StepType::ALGORITHM_PSD_MADS_SUBPROBLEM:
        case NOMAD::StepType::ALGORITHM_PSD_MADS:
        case NOMAD::StepType::ALGORITHM_QPSOLVER:
        case NOMAD::StepType::ALGORITHM_SGTELIB_MODEL:
        case NOMAD::StepType::ALGORITHM_SSD_MADS:
        case NOMAD::StepType::ALGORITHM_DISCO_MADS:
        case NOMAD::StepType::ALGORITHM_QUAD_MODEL:
        case NOMAD::StepType::ALGORITHM_VNS_MADS:
        case NOMAD::StepType::ALGORITHM_RANDOM:
        case NOMAD::StepType::ALGORITHM_COOP_MADS:
            return true;
        default:
            return false;
    }
    return false;   // Only here to avoid compilation warnings
}


std::map<NOMAD::StepType, std::string>& NOMAD::dictStepType()
{
    static std::map<NOMAD::StepType,std::string> dictionary = {
        {NOMAD::StepType::ALGORITHM_LH, "Latin Hypercube"},
        {NOMAD::StepType::ALGORITHM_CS, "Coordinate Search"},
        {NOMAD::StepType::ALGORITHM_DMULTIMADS, "DMultiMADS"},
        {NOMAD::StepType::ALGORITHM_MADS, "MADS"},
        {NOMAD::StepType::ALGORITHM_SIMPLE_MADS, "Simple MADS"},
        {NOMAD::StepType::ALGORITHM_NM, "Nelder-Mead"},
        {NOMAD::StepType::ALGORITHM_PHASE_ONE, "Phase One"},
        {NOMAD::StepType::ALGORITHM_PSD_MADS_SUBPROBLEM, "PSD-Mads subproblem"},
        {NOMAD::StepType::ALGORITHM_PSD_MADS, "PSD-Mads"},
        {NOMAD::StepType::ALGORITHM_QPSOLVER, "Algorithm for Quad Model"},
        {NOMAD::StepType::ALGORITHM_SGTELIB_MODEL, "Sgtelib Model"},
        {NOMAD::StepType::ALGORITHM_SSD_MADS, "SSD-Mads"},
        {NOMAD::StepType::ALGORITHM_DISCO_MADS, "DiscoMads"},
        {NOMAD::StepType::ALGORITHM_RANDOM, "Random algorithm"},
        {NOMAD::StepType::ALGORITHM_QUAD_MODEL, "Quad Model"},
        {NOMAD::StepType::ALGORITHM_VNS_MADS, "VNS Mads"},
        {NOMAD::StepType::ALGORITHM_COOP_MADS, "COOP-Mads"},
        {NOMAD::StepType::INITIALIZATION, "Initialization"},
        {NOMAD::StepType::ITERATION, "Iteration"},
        {NOMAD::StepType::MAIN, "Main"},
        {NOMAD::StepType::MAIN_OBSERVE, "Observe"},
        {NOMAD::StepType::MAIN_SUGGEST, "Suggest"},
        {NOMAD::StepType::MEGA_ITERATION, "MegaIteration"},
        {NOMAD::StepType::MEGA_SEARCH_POLL, "MegaSearchPoll"},

        {NOMAD::StepType::NM_CONTINUE, "NM Continue"},
        {NOMAD::StepType::NM_EXPAND, "NM Expansion"},
        {NOMAD::StepType::NM_INITIAL, "NM Initial step type"},
        {NOMAD::StepType::NM_INITIALIZE_SIMPLEX, "NM Initialize Simplex"},
        {NOMAD::StepType::NM_INSERT_IN_Y, "NM Insert in Y"},
        {NOMAD::StepType::NM_INSIDE_CONTRACTION, "NM Inside Contraction"},
        {NOMAD::StepType::NM_ITERATION, "NM Iteration"},
        {NOMAD::StepType::NM_OUTSIDE_CONTRACTION, "NM Outside Contraction"},
        {NOMAD::StepType::NM_REFLECT, "NM Reflect"},
        {NOMAD::StepType::NM_SHRINK, "NM Shrink"},
        {NOMAD::StepType::NM_UNSET, "NM step type not set"},

        {NOMAD::StepType::MODEL_OPTIMIZE, "Model optimize"},
        {NOMAD::StepType::POLL, "Poll"},
        {NOMAD::StepType::SIMPLE_POLL, "Simple Poll"},
        {NOMAD::StepType::CS_POLL, "Coordinate Search Poll"},
        {NOMAD::StepType::REVEALING_POLL, "Revealing Poll"},
        {NOMAD::StepType::POLL_METHOD_DOUBLE, "Double Poll Method"},
        {NOMAD::StepType::POLL_METHOD_ORTHO_NPLUS1_NEG, "Ortho N+1 Neg Poll Method"},
        {NOMAD::StepType::POLL_METHOD_ORTHO_NPLUS1_QUAD, "Ortho N+1 Quad Poll Method"},
        {NOMAD::StepType::POLL_METHOD_ORTHO_2N, "Ortho 2N Poll Method"},
        {NOMAD::StepType::POLL_METHOD_QR_2N, "QR 2N Poll Method"},
        {NOMAD::StepType::POLL_METHOD_SINGLE, "Single Poll Method"},
        {NOMAD::StepType::POLL_METHOD_UNI_NPLUS1, "Uniform N+1 Poll Method"},
        {NOMAD::StepType::POLL_METHOD_USER, "User-Defined Poll Method"},
        {NOMAD::StepType::CS_POLL_METHOD, "Coordinate Search Poll Method"},
        {NOMAD::StepType::SEARCH, "Search"},
        {NOMAD::StepType::SEARCH_METHOD_ALGO_RANDOM, "Search method using a random algorithm (iteration)"},
        {NOMAD::StepType::SEARCH_METHOD_CACHE, "Cache search method (use to sync algos)"},
        {NOMAD::StepType::SEARCH_METHOD_DMULTIMADS_MIDDLEPOINT, "DMultiMads Middle Point Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_DMULTIMADS_EXPANSIONINT_LINESEARCH, "DMultiMads Expansion integer Linesearch"},
        {NOMAD::StepType::SEARCH_METHOD_DMULTIMADS_QUAD_DMS, "DMultiMads Quad DMS search method"},
        {NOMAD::StepType::SEARCH_METHOD_LH, "Latin Hypercube Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_NM, "Nelder-Mead Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_QUAD_MODEL, "Quadratic Model Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_QUAD_MODEL_SLD, "Quadratic Model (SLD) Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_SGTELIB_MODEL, "Sgtelib Model Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_SIMPLE_RANDOM, "Simple (no iter.) random search method"},
        {NOMAD::StepType::SEARCH_METHOD_SIMPLE_LINE_SEARCH, "Simple line search Method"},
        {NOMAD::StepType::SEARCH_METHOD_SPECULATIVE, "Speculative Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_USER, "User-Defined Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_VNS_MADS, "VNS Mads Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_VNSMART_MADS, "VNSmart Mads Search Method"},
        {NOMAD::StepType::SEARCH_METHOD_REVEALING, "Revealing Search Method"},
        {NOMAD::StepType::SURROGATE_EVALUATION, "Points evaluated using static surrogate"},
        {NOMAD::StepType::MODEL_EVALUATION, "Points evaluated using dynamic model"},
        {NOMAD::StepType::QUAD_MODEL_SORT, "Build dynamic model for sorting points"},
        {NOMAD::StepType::TERMINATION, "Termination"},
        {NOMAD::StepType::UNDEFINED, "Undefined"},
        {NOMAD::StepType::UPDATE, "Update"}
    };

    return dictionary;
}


// Convert a NOMAD::StepType to a string for display.
std::string NOMAD::stepTypeToString(const NOMAD::StepType& stepType)
{
    auto it = dictStepType().find(stepType);
    return it->second;
}


// Convert a vector of StepTypes to a string for display.
std::string NOMAD::StepTypeListToString(const StepTypeList& stepTypeList)
{
    std::string s;

    bool first = true;  // First Step Type to be shown
    for (auto it = stepTypeList.rbegin(); it < stepTypeList.rend(); ++it)
    {
        if (!first)
        {
            s += " - ";
        }
        s += NOMAD::stepTypeToString(*it);
        first = false;
    }
    return s;
}
