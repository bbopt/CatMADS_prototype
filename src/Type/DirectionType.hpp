/**
 \file   DirectionType.hpp
 \brief  Types for Poll direction : Ortho Mads (2n, n+1 Uni, n+1 neg), LT_MADS,
 \author Christophe Tribes
 \date   May 2019
 \see    DirectionType.cpp
 */

#ifndef __NOMAD_4_5_DIRECTION_TYPE__
#define __NOMAD_4_5_DIRECTION_TYPE__

#include <list>
#include <sstream>
#include <vector>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

// Direction type
enum class DirectionType
{
    ORTHO_2N,
    CS,
    ORTHO_NP1_NEG,
    ORTHO_NP1_QUAD,
    QR_2N,
    NP1_UNI,
    SINGLE,
    DOUBLE,
    LT_2N,
    LT_1,
    LT_2,
    LT_NP1,
    GPS_2N_STATIC,
    GPS_2N_RAND,
    GPS_BINARY,
    GPS_NP1_STATIC,
    GPS_NP1_STATIC_UNIFORM,
    GPS_NP1_RAND,
    GPS_NP1_RAND_UNIFORM,
    GPS_1_STATIC,
    USER_POLL,
    USER_FREE_POLL,
    UNDEFINED_DIRECTION
    ///< DirectionType is mandatory
};

typedef std::vector<DirectionType> DirectionTypeList;

/// Convert a list of strings (ex "ORTHO 2N", "ORTHO NP1") to a DirectionType.
DLL_UTIL_API DirectionType stringToDirectionType(const std::list<std::string> & ls);

/// Convert a string (ex "ORTHO 2N", "ORTHO NP1") to a DirectionType.
DLL_UTIL_API DirectionType stringToDirectionType(const std::string & s);

/// Convert an DirectionType to a string
DLL_UTIL_API std::string directionTypeToString(DirectionType dT);
/// Convert a DirectionTypeList to a string
DLL_UTIL_API std::string directionTypeListToString(const DirectionTypeList& dirTypeList);

inline std::ostream& operator<<(std::ostream& out, DirectionType directionType)
{
    out << directionTypeToString(directionType);
    return out;
}


inline std::ostream& operator<<(std::ostream& out, const DirectionTypeList &dirTypeList)
{
    out << directionTypeListToString(dirTypeList);
    return out;
}


#include "../nomad_nsend.hpp"
#endif  // __NOMAD_4_5_DIRECTION_TYPE__
