#ifndef __NOMAD_4_5_DMULTIMADS_SEARCH_STRATEGY_TYPE__
#define __NOMAD_4_5_DMULTIMADS_SEARCH_STRATEGY_TYPE__

#include <string>

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

// Nelder-Mead based search strategies for DMultiMads
enum class DMultiMadsNMSearchType
{
    DOM, ///< Dominance move strategy
    MULTI, ///< MultiMads strategy
};

/// Convert a string (ex "DOM", "MULTI") to a DMultiMadsNMSearchType.
DLL_UTIL_API DMultiMadsNMSearchType stringToDMultiMadsNMSearchType(const std::string& s);

/// Convert a DMultiMadsNMSearchType to a string
DLL_UTIL_API std::string DMultiMadsNMSearchTypeToString(DMultiMadsNMSearchType NMSearchStrategy);

inline std::ostream& operator<<(std::ostream& out, DMultiMadsNMSearchType NMSearchStrategy)
{
    out << DMultiMadsNMSearchTypeToString(NMSearchStrategy);
    return out;
}

// Quadratic-based search strategies for DMultiMads
enum class DMultiMadsQuadSearchType
{
    DMS, ///< DMS strategy
    DOM, ///< DoM strategy
    MULTI ///< MultiMads strategy
};

/// Convert a string (ex "DMS", "DOM", "MULTI") to a DMultiMadsQuadSearchType.
DLL_UTIL_API DMultiMadsQuadSearchType stringToDMultiMadsQuadSearchType(const std::string& s);

/// Convert a DMultiMadsQuadSearchType to a string
DLL_UTIL_API std::string DMultiMadsQuadSearchTypeToString(DMultiMadsQuadSearchType quadSearchStrategy);

inline std::ostream& operator<<(std::ostream& out, DMultiMadsQuadSearchType quadSearchStrategy)
{
    out << DMultiMadsQuadSearchTypeToString(quadSearchStrategy);
    return out;
}

#include "../nomad_nsend.hpp"

#endif //__NOMAD_4_5_DMULTIMADS_SEARCH_STRATEGY_TYPE__
