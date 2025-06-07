/**
 \file   EvalType.cpp
 \brief  types for Eval (implementation)
 \author Viviane Rochon Montplaisir
 \date   November 2019
 \see    EvalType.hpp
 */

#include "../Type/EvalType.hpp"
#include "../Util/Exception.hpp"
#include "../Util/utils.hpp"


// Convert a string ("BB", "MODEL", "SURROGATE")
// to a NOMAD::EvalType.
// If noException is false (default), "UNDEFINED" or "LAST" throws an exception, as well as any value other than "BB", "MODEL" and "SURROGATE".
NOMAD::EvalType NOMAD::stringToEvalType(const std::string &sConst, bool noException )
{
    NOMAD::EvalType ret;
    std::string s = sConst;
    NOMAD::toupper(s);

    if (s == "BB")
    {
        ret = NOMAD::EvalType::BB;
    }
    else if (s == "SURROGATE")
    {
        ret = NOMAD::EvalType::SURROGATE;
    }
    else if (s == "MODEL")
    {
        ret = NOMAD::EvalType::MODEL;
    }
    else
    {
        if (!noException)
        {
            throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized string for NOMAD::EvalType: " + s);
        }
        ret = NOMAD::EvalType::UNDEFINED;
    }

    return ret;
}


// Convert a NOMAD::EvalType to a string.
// NOMAD::EvalType::UNDEFINED returns "UNDEFINED".
// NOMAD::EvalType::LAST throws an exception.
// An unrecognized eval type throws an exception.
std::string NOMAD::evalTypeToString(NOMAD::EvalType evalType)
{
    std::string s;

    switch(evalType)
    {
        case NOMAD::EvalType::BB:
            s = "BB";
            break;
        case NOMAD::EvalType::SURROGATE:
            s = "SURROGATE";
            break;
        case NOMAD::EvalType::MODEL:
            s = "MODEL";
            break;
        case NOMAD::EvalType::UNDEFINED:
            s = "UNDEFINED";
            break;
        case NOMAD::EvalType::LAST:
        default:
            throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized NOMAD::EvalType " + std::to_string((int)evalType));
            break;
    }

    return s;
}

