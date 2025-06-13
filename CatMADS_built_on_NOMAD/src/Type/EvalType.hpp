/**
 \file   EvalType.hpp
 \brief  Types for Evaluation: Blackbox, Model
 \author Viviane Rochon Montplaisir
 \date   November 2019
 \see    EvalType.cpp
 */

#ifndef __NOMAD_4_5_EVAL_TYPE__
#define __NOMAD_4_5_EVAL_TYPE__

#include <sstream>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

// Evaluator type
enum class EvalType
{
    BB,                 ///< The evaluator is a blackbox.
    SURROGATE,          ///< The evaluator is a static surrogate,
                        /// potentially much faster to run than a blackbox.
    MODEL,              ///< The evaluator is a model function,
                        /// potentially much faster to run than a blackbox.
    LAST,               ///< For iterations; Note: UNDEFINED evals are ignored.
    UNDEFINED           ///< Undefined: This value may be used when the
                        ///< EvalType is not mandatory
};


// Convert a string (ex "BB", "MODEL")
// to an EvalType.
DLL_UTIL_API EvalType stringToEvalType(const std::string &s, bool noException = false );

// Convert an EvalType to a string
DLL_UTIL_API std::string evalTypeToString(EvalType evalType);


inline std::ostream& operator<<(std::ostream& out, EvalType evalType)
{
    out << evalTypeToString(evalType);
    return out;
}


#include "../nomad_nsend.hpp"
#endif  // __NOMAD_4_5_EVAL_TYPE__
