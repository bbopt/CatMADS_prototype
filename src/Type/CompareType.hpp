/**
 \file   CompareType.hpp
 \brief  Types for Comparison of objective vector f and h: Blackbox, PhaseOne
 \author Christophe Tribes
 \date   June 2022
 \see    CompareType.cpp
 */

#ifndef __NOMAD_4_5_COMPARE_TYPE__
#define __NOMAD_4_5_COMPARE_TYPE__

#include <sstream>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

// Comparison type
enum class CompareType
{
    EQUAL,              ///< Both points are feasible or infeasible, and their
                        ///< objective values and h (where h is the squared sum
                        ///< of violations of all constraints) are equal to
                        ///< approximation tolerance rounding.
    INDIFFERENT,        ///< Both point are non dominated relatively to each other.
    DOMINATED,          ///< The first point is dominated by the other.
    DOMINATING,         ///< The first point dominates the other.
    UNDEFINED           ///< May be used when comparing feasible and infeasible solutions for example.
};

// Convert a string (ex "EQUAL", "INDIFFERENT")
// to a CompareType.
DLL_UTIL_API CompareType stringToCompareType(const std::string &s);

// Convert an CompareType to a string
DLL_UTIL_API std::string compareTypeToString (CompareType compareType);


inline std::ostream& operator<<(std::ostream& out, CompareType compareType)
{
    out << compareTypeToString(compareType);
    return out;
}


#include "../nomad_nsend.hpp"
#endif  // __NOMAD_4_5_COMPARE_TYPE__
