/**
 \file   EvalSortType.cpp
 \brief  types for sorting EvalPoints (implementation)
 \author Viviane Rochon Montplaisir
 \date   April 2021
 \see    EvalSortType.hpp
 */

#include "../Type/EvalSortType.hpp"
#include "../Util/Exception.hpp"
#include "../Util/utils.hpp"


// Convert a string ("LEXICOGRAPHICAL", "DIR_LAST_SUCCESS", "RANDOM", "SURROGATE", "QUADRATIC_MODEL", "USER")
// to a NOMAD::EvalSortType.
NOMAD::EvalSortType NOMAD::stringToEvalSortType(const std::string &sConst)
{
    NOMAD::EvalSortType ret;
    std::string s = sConst;
    NOMAD::toupper(s);

    if (s == "DIR_LAST_SUCCESS")
    {
        ret = NOMAD::EvalSortType::DIR_LAST_SUCCESS;
    }
    else if (s == "LEXICOGRAPHICAL")
    {
        ret = NOMAD::EvalSortType::LEXICOGRAPHICAL;
    }
    else if (s == "RANDOM")
    {
        ret = NOMAD::EvalSortType::RANDOM;
    }
    else if (s == "SURROGATE")
    {
        ret = NOMAD::EvalSortType::SURROGATE;
    }
    else if (s == "QUADRATIC_MODEL")
    {
        ret = NOMAD::EvalSortType::QUADRATIC_MODEL;
    }
    else if (s == "USER")
    {
        ret = NOMAD::EvalSortType::USER;
    }
    else
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized string for NOMAD::EvalSortType: " + s);
    }

    return ret;
}


// Convert a NOMAD::EvalSortType to a string.
// An unrecognized EvalSortType returns an exception.
std::string NOMAD::evalSortTypeToString(const NOMAD::EvalSortType& evalSortType)
{
    std::string s;

    switch(evalSortType)
    {
        case NOMAD::EvalSortType::DIR_LAST_SUCCESS:
            s = "DIR_LAST_SUCCESS";
            break;
        case NOMAD::EvalSortType::LEXICOGRAPHICAL:
            s = "LEXICOGRAPHICAL";
            break;
        case NOMAD::EvalSortType::RANDOM:
            s = "RANDOM";
            break;
        case NOMAD::EvalSortType::SURROGATE:
            s = "SURROGATE";
            break;
        case NOMAD::EvalSortType::QUADRATIC_MODEL:
            s = "QUADRATIC_MODEL";
            break;
        case NOMAD::EvalSortType::USER:
            s = "USER";
            break;
        default:
            throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized NOMAD::EvalSortType " + std::to_string((int)evalSortType));
            break;
    }

    return s;
}

