/**
 \file   BBInputType.hpp
 \brief  Types for BBInput
 \author Viviane Rochon Montplaisir
 \date   December 2018
 \see    BBInputType.cpp
 */


#ifndef __NOMAD_4_5_BB_INPUT_TYPE__
#define __NOMAD_4_5_BB_INPUT_TYPE__

#include <sstream>
#include <vector>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

/// Enum for blackbox input type
/**
 Related to problem formulation
 \note Categorical variables not supported yet.
*/
enum class BBInputType
{
    CONTINUOUS  ,     ///< Continuous variable (default) (R)
    ALL_CONTINUOUS  , ///< All variables are continuous variable (default, *R). Need a checkAndComply to set the BBInputTypeList.
    INTEGER     ,     ///< Integer variable (I)
    ALL_INTEGER ,     ///< All variables are integer (*I). Need a checkAndComply to set the BBInputTypeList.
    //CATEGORICAL ,   ///< Categorical variable          (C)
    BINARY         ,  ///< Binary variable               (B)
    ALL_BINARY       ///< All variables are binary (*B). Need a checkAndComply to set the BBInputTypeList.
};


typedef std::vector<BBInputType> BBInputTypeList;
typedef std::vector<BBInputType>::const_iterator BBInputTypeListIt;


/// Utility for BBInputTypes.
/**
 Convert a string ("R", "*R", "I", "*I", "B", "*B") to a blackbox input type.
 */
DLL_UTIL_API BBInputType stringToBBInputType(const std::string &s);

/// Utility for BBInputTypes
/**
 * Convert a string containing multiple BBInputTypes (ex "( R I B R )") to a BBInputTypeList or a single 'all of the same type (*)' BBInputType to BBInputTypeList of a single element. When have a single 'all of the same type (*)', the list of full dimension is created when calling PbParameters::checkAndComply. \n
 *
 *\todo Support the syntax 1-4 I
 *
 */
DLL_UTIL_API BBInputTypeList stringToBBInputTypeList(const std::string &s);


inline std::ostream& operator<<(std::ostream& out, const BBInputType &bbinputtype)
{
    switch(bbinputtype)
    {
        case BBInputType::CONTINUOUS:
            out << "R";
            break;
        case BBInputType::INTEGER:
            out << "I";
            break;
        case BBInputType::BINARY:
            out << "B";
            break;
        default:
            out << "R"; // Unrecognized, output default: CONTINUOUS
            break;
    }
    return out;
}

/// Display a list of blackbox input type.
inline std::ostream& operator<<(std::ostream& out, const BBInputTypeList &bbinputtypelist)
{
    BBInputTypeListIt it;
    bool first = true;
    for (it = bbinputtypelist.begin(); it != bbinputtypelist.end(); ++it)
    {
        if (!first)
        {
            out << " ";
        }
        out << *it;
        first = false;
    }
    return out;
}

#include "../nomad_nsend.hpp"
#endif  // __NOMAD_4_5_BB_INPUT_TYPE__
