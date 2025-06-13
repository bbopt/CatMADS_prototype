/**
 \file   CacheBase.cpp
 \brief  Code for base class CacheBase
 \author Viviane Rochon Montplaisir
 \date   February 2019
 \see    CacheBase.hpp
 */

#include "../Cache/CacheBase.hpp"
#include "../Util/fileutils.hpp"


// Initialize CacheBase class.
// To be called by the Constructor.
void NOMAD::CacheBase::init()
{
    // Default cache parameters are considered if cacheParams is not set
    if (nullptr == _cacheParams)
    {
        _cacheParams = std::make_shared<CacheParameters>();
    }

    _maxSize  = _cacheParams->getAttributeValue<size_t>("CACHE_SIZE_MAX") ;
    _filename = _cacheParams->getAttributeValue<std::string>("CACHE_FILE");
    // Verify filename has full path, otherwise, confusion will arise
    if (!_filename.empty() && !NOMAD::isAbsolute(_filename))
    {
        std::string err = "Error: Cache file name should have been converted to full path: ";
        err += _filename;
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }
}


bool isTrue(const NOMAD::EvalPoint& NOMAD_UNUSED(evalPoint))
{
    return true;
}


size_t NOMAD::CacheBase::getAllPoints(std::vector<NOMAD::EvalPoint> &evalPointList) const
{
    evalPointList.clear();
    return find(isTrue, evalPointList);
}

