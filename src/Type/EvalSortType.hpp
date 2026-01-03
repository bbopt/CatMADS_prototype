/**
 \file   EvalSortType.hpp
 \brief  Types for sorting point before evaluation: DirLastSuccess, Lexicographical, Random
 \author Viviane Rochon Montplaisir
 \date   April 2021
 \see    EvalSortType.cpp
 */

#ifndef __NOMAD_4_5_EVAL_SORT_TYPE__
#define __NOMAD_4_5_EVAL_SORT_TYPE__

#include <sstream>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

// Evaluator type
enum class EvalSortType
{
    DIR_LAST_SUCCESS,   ///< Sort using direction of last successful point
    LEXICOGRAPHICAL,    ///< Sort using lexicographical order
    RANDOM,             ///< Mix points randomly instead of sorting them
    SURROGATE,          ///< Sort using static surrogate evaluator
    QUADRATIC_MODEL,    ///< Sort using dynamic quadratic model
    USER                ///< Sort comparison operator provided by user
};


// Convert a string (ex "DIR_LAST_SUCCESS", "LEXICOGRAPHICAL")
// to an EvalSortType.
DLL_UTIL_API EvalSortType stringToEvalSortType(const std::string &s);

// Convert an EvalSortType to a string
DLL_UTIL_API std::string evalSortTypeToString(const EvalSortType& evalSortType);


inline std::ostream& operator<<(std::ostream& out, EvalSortType evalSortType)
{
    out << evalSortTypeToString(evalSortType);
    return out;
}


#include "../nomad_nsend.hpp"
#endif  // __NOMAD_4_5_EVAL_SORT_TYPE__
