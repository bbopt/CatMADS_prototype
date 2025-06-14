/**
 \file   DirectionType.cpp
 \brief  types for Direction (implementation)
 \author Christophe Tribes
 \date   May 2020
 \see    DirectionType.hpp
 */

#include "../Type/DirectionType.hpp"
#include "../Util/Exception.hpp"
#include "../Util/utils.hpp"
#include <numeric>    // For std::accumulate

// Convert a string with separation ("ORTHO 2N", "ORTHO N+1 NEG", ...)
// to a NOMAD::DirectionType.
NOMAD::DirectionType NOMAD::stringToDirectionType(const std::string & s)
{
    std::list<std::string> ls;
    std::size_t current, previous = 0;
    current = s.find(' ');
    while (current != std::string::npos)
    {
        ls.push_back(s.substr(previous, current - previous));
        previous = current + 1;
        current = s.find(' ', previous);
    }
    ls.push_back(s.substr(previous, current - previous));
    return stringToDirectionType(ls);
}

// Convert a list of strings ("ORTHO 2N", "ORTHO N+1 NEG", ..., "QR 2N")
// to a NOMAD::DirectionType.
NOMAD::DirectionType NOMAD::stringToDirectionType(const std::list<std::string> & ls)
{
    NOMAD::DirectionType ret=DirectionType::UNDEFINED_DIRECTION;

    if ( ls.empty() )
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "An empty list of string cannot be converted to NOMAD::DirectionType ");
    }
    if ( ls.size() > 4 )
    {
        std::string err = "List of strings cannot be converted to NOMAD::DirectionType: ";
        err += std::accumulate(ls.begin(), ls.end(), std::string(" "));
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }

    auto it = ls.begin() , end = ls.end();
    std::string                            s  = *it;
    NOMAD::toupper ( s );

    // NP1 UNIFORM direction
    if (s == "N+1")
    {
        ++it;
        if (*it == "UNI")
            ret = NOMAD::DirectionType::NP1_UNI;
    }

    // SINGLE direction
    if (s == "SINGLE")
    {
        ret = NOMAD::DirectionType::SINGLE;
    }

    // DOUBLE direction
    if (s == "DOUBLE")
    {
        ret = NOMAD::DirectionType::DOUBLE;
    }

    if (s == "CS")
    {
        ret = NOMAD::DirectionType::CS;
    }

    if (s == "USER_POLL")
    {
        ret = NOMAD::DirectionType::USER_POLL;
    }

    if (s == "USER_FREE_POLL")
    {
        ret = NOMAD::DirectionType::USER_FREE_POLL;
    }

    // Ortho-MADS with n+1 (plus QUAD or NEG), or 2n directions:
    if (s == "ORTHO")
    {
        ++it;
        if ( it == end )
        {
            ret = NOMAD::DirectionType::ORTHO_NP1_QUAD;  // Default for ORTHO
        }
        if ( *it == "1" )
        {
            ret = NOMAD::DirectionType::SINGLE; // For retro compatibility with Nomad 3
        }
        if ( *it == "2" )
        {
            ret = NOMAD::DirectionType::DOUBLE; // For retro compatibility with Nomad 3
        }
        s = *it;
        NOMAD::toupper ( s );
        if ( s == "2N" )
        {
            ret = NOMAD::DirectionType::ORTHO_2N;
        }
        if ( s == "N+1" )
        {
            ++it;
            if (it==end)
            {
                throw NOMAD::Exception(__FILE__, __LINE__, "ORTHO N+1 QUAD direction type not yet supported");
                ret = NOMAD::DirectionType::ORTHO_NP1_QUAD;   // Default for ORTHO N+1
            }
            s = *it;
            NOMAD::toupper ( s );
            if ( s=="QUAD" )
            {
                ret= NOMAD::DirectionType::ORTHO_NP1_QUAD;
            }
            if ( s=="NEG" )
            {
                ret=NOMAD::DirectionType::ORTHO_NP1_NEG;
            }
            if ( s=="UNI" )
            {
                ret=NOMAD::DirectionType::NP1_UNI; // For retro compatibility with Nomad 3
            }

        }
    }

    if (s == "QR")
    {
        ++it;
        if ( it == end )
        {
            ret = NOMAD::DirectionType::QR_2N;  // Default for ORTHO
        }
        s = *it;
        NOMAD::toupper ( s );
        if ( s == "2N" )
        {
            ret = NOMAD::DirectionType::QR_2N;
        }
// Not yet supported
//        if ( s == "N+1" )
//        {
//            ++it;
//            if (it==end)
//            {
//                throw NOMAD::Exception(__FILE__, __LINE__, "QR N+1 QUAD direction type not yet supported");
//                ret = NOMAD::DirectionType::ORTHO_NP1_QUAD;   // Default for ORTHO N+1
//            }
//            s = *it;
//            NOMAD::toupper ( s );
//            if ( s=="QUAD" )
//            {
//                ret= NOMAD::DirectionType::QR_NP1_QUAD;
//            }
//            if ( s=="NEG" )
//            {
//                ret=NOMAD::DirectionType::QR_NP1_NEG;
//            }
//        }
    }


    // LT-MADS with 1, 2 or 2n directions:
    if ( s == "LT" )
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "LT direction type not yet supported");

// TODO
//        ++it;
//        if ( it == end )
//        {
//            ret = NOMAD::DirectionType::LT_2N;
//        }
//        if ( *it == "1" )
//        {
//            ret = NOMAD::DirectionType::LT_1;
//        }
//        if ( *it == "2" )
//        {
//            ret = NOMAD::DirectionType::LT_2;
//        }
//        s = *it;
//        NOMAD::toupper ( s );
//        if ( s == "N+1" )
//        {
//            ret = NOMAD::DirectionType::LT_NP1;
//        }
//        if ( s == "2N" )
//        {
//            ret = NOMAD::DirectionType::LT_2N;
//        }
    }

    // GPS:
    if ( s == "GPS" )
    {

        throw NOMAD::Exception(__FILE__, __LINE__, "GPS direction type not yet supported");
// TODO
//        ++it;
//        if ( it == end )
//        {
//            ret = NOMAD::DirectionType::GPS_2N_STATIC;
//        }
//        s = *it;
//        NOMAD::toupper ( s );
//
//        // GPS for binary variables:
//        if ( s == "BINARY" || s == "BIN" )
//        {
//            ret = NOMAD::DirectionType::GPS_BINARY;
//        }
//
//        // GPS, n+1 directions:
//        if ( s == "N+1" )
//        {
//            ++it;
//            if ( it == end )
//            {
//                ret = NOMAD::DirectionType::GPS_NP1_STATIC;
//            }
//            s = *it;
//            NOMAD::toupper ( s );
//
//            // GPS, n+1, static:
//            if ( s == "STATIC" )
//            {
//                ++it;
//                if ( it == end )
//                {
//                    ret = NOMAD::DirectionType::GPS_NP1_STATIC;
//                }
//                s = *it;
//                NOMAD::toupper ( s );
//                if ( s == "UNIFORM" )
//                {
//                    ret = NOMAD::DirectionType::GPS_NP1_STATIC_UNIFORM;
//                }
//            }
//
//            // GPS, n+1, random:
//            if ( s == "RAND" || s == "RANDOM" )
//            {
//                ++it;
//                if ( it == end )
//                {
//                    ret = NOMAD::DirectionType::GPS_NP1_RAND;
//                }
//                s = *it;
//                NOMAD::toupper ( s );
//                if ( s == "UNIFORM" )
//                {
//                    ret = NOMAD::DirectionType::GPS_NP1_RAND_UNIFORM;
//                }
//            }
//        }
//
//        // 2n directions:
//        if ( s == "2N" )
//        {
//            ++it;
//            if ( it == end )
//            {
//                ret = NOMAD::DirectionType::GPS_2N_STATIC;
//            }
//            s = *it;
//            NOMAD::toupper ( s );
//            if ( s == "STATIC" )
//            {
//                ret = NOMAD::DirectionType::GPS_2N_STATIC;
//            }
//            if ( s == "RAND" || s == "RANDOM" )
//            {
//                ret = NOMAD::DirectionType::GPS_2N_RAND;
//            }
//        }
//        // 2n directions:
//        if ( s == "1" )
//        {
//            ++it;
//            if ( it == end )
//            {
//                ret = NOMAD::DirectionType::GPS_1_STATIC;
//            }
//            s = *it;
//            NOMAD::toupper ( s );
//            if ( s == "STATIC" )
//            {
//                ret = NOMAD::DirectionType::GPS_1_STATIC;
//            }
//        }
    }

    if (ret == DirectionType::UNDEFINED_DIRECTION)
    {
        std::string err = "List of strings cannot be converted to NOMAD::DirectionType ";
        err += std::accumulate(ls.begin(), ls.end(), std::string(" "));
        throw NOMAD::Exception(__FILE__, __LINE__, err );
    }

    return ret;
}


// Convert a NOMAD::DirectionType to a string.
// NOMAD::DirectionType::UNDEFINED returns "UNDEFINED".
// An unrecognized eval type returns an exception.
std::string NOMAD::directionTypeToString(NOMAD::DirectionType dt)
{
    std::string s;

    switch(dt)
    {
        case NOMAD::DirectionType::ORTHO_2N:
            s = "ORTHO 2N";
            break;
        case NOMAD::DirectionType::CS:
            s = "CS";
            break;
        case NOMAD::DirectionType::NP1_UNI:
            s = "N+1 UNI";
            break;
        case NOMAD::DirectionType::ORTHO_NP1_NEG:
            s = "ORTHO N+1 NEG";
            break;
        case NOMAD::DirectionType::ORTHO_NP1_QUAD:
            s = "ORTHO N+1 QUAD";
            break;
        case NOMAD::DirectionType::QR_2N:
            s = "QR 2N";
            break;
        case NOMAD::DirectionType::SINGLE:
            s = "SINGLE";
            break;
        case NOMAD::DirectionType::DOUBLE:
            s = "DOUBLE";
            break;
        case NOMAD::DirectionType::USER_POLL:
            s = "USER_POLL";
            break;
        case NOMAD::DirectionType::USER_FREE_POLL:
            s = "USER_FREE_POLL";
            break;
        default:
            throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized NOMAD::DirectionType " + std::to_string((int)dt));
            break;
    }

    return s;
}


std::string NOMAD::directionTypeListToString(const DirectionTypeList& dirTypeList)
{
    std::string s;

    bool first = true;
    for (auto dirType : dirTypeList)
    {
        if (!first)
        {
            s += " ; ";
        }
        s += NOMAD::directionTypeToString(dirType);
        first = false;
    }

    return s;
}
