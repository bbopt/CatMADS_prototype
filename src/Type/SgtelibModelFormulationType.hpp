/**
 \file   SgtelibModelFormulationType.hpp
 \brief  types for parameter SGTELIB_MODEL_FORMULATION
 \author Viviane Rochon Montplaisir
 \date   July 2019
 \see    SgtelibModel.hpp
 */
#ifndef __NOMAD_4_5_SGTELIB_MODEL_FORMULATION_TYPE__
#define __NOMAD_4_5_SGTELIB_MODEL_FORMULATION_TYPE__

#include <string>
#include <sstream>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"


/// Formulations for sgtelib model Search
enum class SgtelibModelFormulationType
{
    FS    ,  /// min f-lambda*sigma, st c-lambda*sigma < 0
    FSP   ,  /// min f-lambda*sigma, st P(x) > 1/2
    EIS   ,  /// min -EI-lambda*sigma, st c-lambda*sigma < 0
    EFI   ,  /// min -EFI
    EFIS  ,  /// min -EFI-lambda*sigma
    EFIM  ,  /// min -EFI-lambda*sigma*mu
    EFIC  ,  /// min -EFI-lambda*(EI*sigma+P*mu)
    PFI   ,  /// min -PFI
    D     ,  /// min -distance_to_closest
    EXTERN,  /// min f, st c, with extern executable model
    UNDEFINED /// Undefined
};


// Convert a string (ex "FS", "EIS", "FSP"...)
// to a SgtelibModelFormulationType.
DLL_UTIL_API SgtelibModelFormulationType stringToSgtelibModelFormulationType(const std::string &s);

DLL_UTIL_API std::string SgtelibModelFormulationTypeToString(const SgtelibModelFormulationType &smft);


inline std::ostream& operator<<(std::ostream& os, const SgtelibModelFormulationType &smft)
{
    switch (smft)
    {
        case SgtelibModelFormulationType::FS:
            os << "FS";
            break;
        case SgtelibModelFormulationType::FSP:
            os << "FSP";
            break;
        case SgtelibModelFormulationType::EIS:
            os << "EIS";
            break;
        case SgtelibModelFormulationType::EFI:
            os << "EFI";
            break;
        case SgtelibModelFormulationType::EFIS:
            os << "EFIS";
            break;
        case SgtelibModelFormulationType::EFIM:
            os << "EFIM";
            break;
        case SgtelibModelFormulationType::EFIC:
            os << "EFIC";
            break;
        case SgtelibModelFormulationType::PFI:
            os << "PFI";
            break;
        case SgtelibModelFormulationType::D:
            os << "D";
            break;
        case SgtelibModelFormulationType::EXTERN:
            os << "EXTERN";
            break;
        default:
            return os << "UNDEFINED";
            break;
    }

    return os;
}



#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SGTELIB_MODEL_FORMULATION_TYPE__
