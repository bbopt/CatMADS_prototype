/**
 \file   ComputeType.cpp
 \brief  Types for Computation of f and h (implementation)
 \author Viviane Rochon Montplaisir
 \date   February 2021
 \see    ComputeType.hpp
 */

#include "../Type/ComputeType.hpp"
#include "../Util/Exception.hpp"
#include "../Util/utils.hpp"


// Convert a string (Ex. "STANDARD", "PHASE_ONE", "USER")
// to a NOMAD::ComputeType.
// "UNDEFINED" throws an exception, as well as any value other than "STANDARD", "PHASE_ONE", "USER".
NOMAD::ComputeType NOMAD::stringToComputeType(const std::string &sConst)
{
    NOMAD::ComputeType ret;
    std::string s = sConst;
    NOMAD::toupper(s);

    if (s == "STANDARD")
    {
        ret = NOMAD::ComputeType::STANDARD;
    }
    else if (s == "PHASE_ONE")
    {
        ret = NOMAD::ComputeType::PHASE_ONE;
    }
    else if (s == "DMULTI_COMBINE_F")
    {
        ret = NOMAD::ComputeType::DMULTI_COMBINE_F;
    }
    else if (s == "USER")
    {
        ret = NOMAD::ComputeType::USER;
        throw NOMAD::Exception(__FILE__, __LINE__, "User ComputeType is not available");
    }
    else
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized string for NOMAD::ComputeType: " + s);
    }

    return ret;
}


// Convert a NOMAD::ComputeType to a string.
// NOMAD::ComputeType::UNDEFINED returns "UNDEFINED".
// An unrecognized compute type returns an exception.
std::string NOMAD::computeTypeToString(NOMAD::ComputeType computeType)
{
    std::string s;

    switch(computeType)
    {
        case NOMAD::ComputeType::STANDARD:
            s = "STANDARD";
            break;
        case NOMAD::ComputeType::PHASE_ONE:
            s = "PHASE_ONE";
            break;
        case NOMAD::ComputeType::DMULTI_COMBINE_F:
            s = "DMULTI_COMBINE_F";
            break;
        case NOMAD::ComputeType::USER:
            s = "USER";
            break;
        case NOMAD::ComputeType::UNDEFINED:
            s = "UNDEFINED";
            break;
        default:
            throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized NOMAD::ComputeType " + std::to_string((int)computeType));
            break;
    }

    return s;
}

// Convert a string ("L1", "L2", "Linf") to a NOMAD::HNormType.
NOMAD::HNormType NOMAD::stringToHNormType(const std::string &sConst)
{
    NOMAD::HNormType ret;
    std::string s = sConst;
    NOMAD::toupper(s);

    if (s == "L1")
    {
        ret = NOMAD::HNormType::L1;
    }
    else if (s == "L2")
    {
        ret = NOMAD::HNormType::L2;
    }
    else if (s == "Linf")
    {
        ret = NOMAD::HNormType::Linf;
    }
    else
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized string for NOMAD::HNormType: " + s);
    }

    return ret;
}


// Convert a NOMAD::HNormType to a string.
// An unrecognized type throws an exception.
std::string NOMAD::hNormTypeToString(NOMAD::HNormType hNormType)
{
    std::string s;

    switch(hNormType)
    {
        case NOMAD::HNormType::L1:
            s = "L1";
            break;
        case NOMAD::HNormType::L2:
            s = "L2";
            break;
        case NOMAD::HNormType::Linf:
            s = "Linf";
            break;
        default:
            throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized NOMAD::HNormType " + std::to_string((int)hNormType));
            break;
    }

    return s;
}
