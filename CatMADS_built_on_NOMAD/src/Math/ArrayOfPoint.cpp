/**
 \file   ArrayOfPoint.cpp
 \brief  Vector of NOMAD::Points (implementation)
 \author Viviane Rochon Montplaisir
 \date   August 2019
 \see    ArrayOfPoint.hpp
 */
#include "../Math/ArrayOfPoint.hpp"



std::ostream& NOMAD::operator<< (std::ostream& out, const NOMAD::ArrayOfPoint& aop)
{
    for (size_t i = 0; i < aop.size(); i++)
    {
        if (i > 0)
        {
            out << " ";
        }
        out << aop[i].display();
    }
    return out;
}


std::istream& NOMAD::operator>>(std::istream& in, ArrayOfPoint& aop)
{
    // Patch: The ArrayOfPoint must contain 1 Point with the right dimension.
    if (aop.empty() || 0 == aop[0].size())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Input ArrayOfPoint should have a point of nonzero value");
    }

    size_t n = aop[0].size();
    aop.clear();
    NOMAD::ArrayOfDouble aod(n);
    NOMAD::Point point(n);

    // Usually, point files do not have parenthesis in them, so read
    // points as ArrayOfDouble. If Point parenthesis is needed, we can add it.
    while (in >> aod && in.good() && !in.eof())
    {
        point = aod;
        aop.push_back(point);
    }
    if (!in.eof() || !point.isComplete())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Error while reading point file. A carriage return maybe be required at the end of a line.");
    }

    return in;
}

