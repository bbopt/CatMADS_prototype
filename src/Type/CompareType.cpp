/**
 \file   CompareType.cpp
 \brief  Types for Comparison of objective vector f and h (implementation)
 \author Ludovic Salomon
 \date   February 2022
 \see    CompareType.hpp
 */

#include "../Type/CompareType.hpp"
#include "../Util/Exception.hpp"
#include "../Util/utils.hpp"


// Convert a string (Ex. "EQUAL", "INDIFFERENT", "DOMINATED", "DOMINATING")
// to a NOMAD::CompareType.
// "UNDEFINED" throws an exception, as well as any value other than "EQUAL", "INDIFFERENT",
// "DOMINATED", "DOMINATING".
NOMAD::CompareType NOMAD::stringToCompareType(const std::string &sConst)
{
    NOMAD::CompareType ret;
    std::string s = sConst;
    NOMAD::toupper(s);

    if (s == "EQUAL")
    {
        ret = NOMAD::CompareType::EQUAL;
    }
    else if (s == "INDIFFERENT")
    {
        ret = NOMAD::CompareType::INDIFFERENT;
    }
    else if (s == "DOMINATED")
    {
        ret = NOMAD::CompareType::DOMINATED;
    }
    else if (s == "DOMINATING")
    {
        ret = NOMAD::CompareType::DOMINATING;
    }
    else
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized string for NOMAD::CompareType: " + s);
    }

    return ret;
}


// Convert a NOMAD::CompareType to a string.
// NOMAD::CompareType::UNDEFINED returns "UNDEFINED".
// An unrecognized compute type returns an exception.
std::string NOMAD::compareTypeToString(NOMAD::CompareType compareType)
{
    std::string s;

    switch(compareType)
    {
        case NOMAD::CompareType::EQUAL:
            s = "EQUAL";
            break;
        case NOMAD::CompareType::INDIFFERENT:
            s = "INDIFFERENT";
            break;
        case NOMAD::CompareType::DOMINATED:
            s = "DOMINATED";
            break;
        case NOMAD::CompareType::DOMINATING:
            s = "DOMINATING";
            break;
        case NOMAD::CompareType::UNDEFINED:
            s = "UNDEFINED";
            break;
        default:
            throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized NOMAD::CompareType " + std::to_string((int)compareType));
            break;
    }

    return s;
}

