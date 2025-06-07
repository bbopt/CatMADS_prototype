#include "DMultiMadsSearchStrategyType.hpp"

#include "../Util/Exception.hpp"
#include "../Util/utils.hpp"

// Convert a string ("DOM", "MULTI") to a NOMAD::DMultiMadsNMSearchType.
NOMAD::DMultiMadsNMSearchType NOMAD::stringToDMultiMadsNMSearchType(const std::string& sConst)
{
    std::string s = sConst;
    NOMAD::toupper(s);

    if (s == "DOM")
    {
        return NOMAD::DMultiMadsNMSearchType::DOM;
    }
    else if (s == "MULTI")
    {
        return NOMAD::DMultiMadsNMSearchType::MULTI;
    }
    else
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized string for NOMAD::DMultiMadsNMSearchType: " + s);
    }

    return NOMAD::DMultiMadsNMSearchType::DOM;
}

// Convert a NOMAD::DMultiMadsNMSearchType to a string
std::string NOMAD::DMultiMadsNMSearchTypeToString(NOMAD::DMultiMadsNMSearchType NMSearchStrategy)
{
    if (NMSearchStrategy == DMultiMadsNMSearchType::DOM)
    {
        return "DOM";
    }
    else if (NMSearchStrategy == DMultiMadsNMSearchType::MULTI)
    {
        return "MULTI";
    }
    else
    {
        return "UNDEFINED";
    }
}

// Convert a string ("DOM", "MULTI") to a NOMAD::DMultiMadsQuadSearchType.
NOMAD::DMultiMadsQuadSearchType NOMAD::stringToDMultiMadsQuadSearchType(const std::string& sConst)
{
    std::string s = sConst;
    NOMAD::toupper(s);

    if (s == "DMS")
    {
        return NOMAD::DMultiMadsQuadSearchType::DMS;
    }
    else if (s == "DOM")
    {
        return NOMAD::DMultiMadsQuadSearchType::DOM;
    }
    else if (s == "MULTI")
    {
        return NOMAD::DMultiMadsQuadSearchType::MULTI;
    }
    else
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Unrecognized string for NOMAD::DMultiMadsQuadSearchType: " + s);
    }

    return NOMAD::DMultiMadsQuadSearchType::DMS;
}

// Convert a NOMAD::DMultiMadsNMSearchType to a string
std::string NOMAD::DMultiMadsQuadSearchTypeToString(NOMAD::DMultiMadsQuadSearchType quadSearchStrategy)
{
    if (quadSearchStrategy == DMultiMadsQuadSearchType::DMS)
    {
        return "DMS";
    }
    else if (quadSearchStrategy == DMultiMadsQuadSearchType::DOM)
    {
        return "DOM";
    }
    else if (quadSearchStrategy == DMultiMadsQuadSearchType::MULTI)
    {
        return "MULTI";
    }
    else
    {
        return "UNDEFINED";
    }
}
