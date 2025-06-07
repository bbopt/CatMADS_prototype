/**
 \file   ComputeType.hpp
 \brief  Types for Computation of f and h: Blackbox, PhaseOne
 \author Viviane Rochon Montplaisir
 \date   February 2021
 \see    ComputeType.cpp
 */

#ifndef __NOMAD_4_5_COMPUTE_TYPE__
#define __NOMAD_4_5_COMPUTE_TYPE__

#include "../Eval/BBOutput.hpp"
#include "../Type/BBOutputType.hpp"
#include "../Type/EvalType.hpp"
#include "../Math/ArrayOfDouble.hpp"

#include <functional>
#include <sstream>
#include <functional>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"


// Computation type for f and h
enum class ComputeType
{
    STANDARD,           ///< f is OBJ. h is a norm of the violations of
                        ///< all constraints. EB constraint violation result
                        ///< in h being INF.
    PHASE_ONE,          ///< f is computed based on the violation of EB
                        ///< constraints only. h is always 0. OBJ and PB
                        ///< constraints are ignored.
    DMULTI_COMBINE_F,   ///< Combined f is computed from all f values
    USER,               ///< f and h are computed by two user-provided functions.
    UNDEFINED           ///< Undefined: This value may be used when the
                        ///< ComputeType is not mandatory
};

// Convert a string (ex "STANDARD", "PHASE_ONE")
// to an ComputeType.
DLL_UTIL_API ComputeType stringToComputeType(const std::string &s);

// Convert an ComputeType to a string
DLL_UTIL_API std::string computeTypeToString (ComputeType computeType);


inline std::ostream& operator<<(std::ostream& out, ComputeType computeType)
{
    out << computeTypeToString(computeType);
    return out;
}

// Evaluator HNorm type
enum class HNormType
{
    L1,
    L2,
    Linf
};

// Convert a string to an HNormType.
DLL_UTIL_API HNormType stringToHNormType(const std::string &s);

// Convert an HNormType to a string
DLL_UTIL_API std::string hNormTypeToString(HNormType evalType);


inline std::ostream& operator<<(std::ostream& out, HNormType evalType)
{
    out << hNormTypeToString(evalType);
    return out;
}

// Type for single ouput compute function (single F or H). For now it is used only for User defined F and H compute OR DMultiMads with NM search: multi objs are combined into a single obj.
typedef std::function<Double(const BBOutputTypeList &bbOutputTypeList , const BBOutput & bbOutput)> singleOutputComputeFType;

// Default empty single output compute function is defined for convenience. It is replaced by the default single objective/infeasibilityH compute function when setting compute type.
// For now a non-default, non-empty function is passed only for DMultiMads with NM search or QuadModelSearch.
const singleOutputComputeFType defaultEmptySingleOutputCompute = [](const BBOutputTypeList &bbOutputTypeList , const BBOutput & bbOutput) -> Double{ return Double(); };

// FH compute type without EvalType
struct FHComputeTypeS {
    ComputeType computeType = ComputeType::STANDARD;
    HNormType hNormType = HNormType::L2;
    singleOutputComputeFType singleObjectiveCompute = defaultEmptySingleOutputCompute;
    singleOutputComputeFType infeasHCompute = defaultEmptySingleOutputCompute;
};

// Default is taken from struct initializers
const FHComputeTypeS defaultFHComputeTypeS;


// Complete info to compute F and H (eval type, compute type and h norm type)
struct FHComputeType {
    EvalType evalType = EvalType::BB;
    FHComputeTypeS fhComputeTypeS;

    // Access to the short version of FHComputeType
    // Used when eval type is known
    NOMAD::FHComputeTypeS Short() const
    {
        return fhComputeTypeS;
    }

};

// Default taken from struct initializer
const FHComputeType defaultFHComputeType;


#include "../nomad_nsend.hpp"
#endif  // __NOMAD_4_5_COMPUTE_TYPE__
