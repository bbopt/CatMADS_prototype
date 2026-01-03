
#include "../Math/LHS.hpp"
#include "../Math/RNG.hpp"
#include "../Math/RandomPickup.hpp"
#include "../Util/Exception.hpp"

#include <algorithm>    // for shuffle


// Constructor
NOMAD::LHS::LHS(const size_t n,
                const size_t p,
                const NOMAD::ArrayOfDouble& lowerBound,
                const NOMAD::ArrayOfDouble& upperBound,
                const NOMAD::Point& frameCenter,
                const NOMAD::ArrayOfDouble& deltaFrameSize,
                const NOMAD::Double& scaleFactor)
:   _n(n),
    _p(p),
    _lowerBound(lowerBound),
    _upperBound(upperBound)
{
    // Update undefined values of lower and upper bounds to use values based
    // on deltaFrameSize.
    // Based on the code in NOMAD 3, but slightly different.
    // Do not use INF values for bounds, that will generate points with huge
    // values. It is not elegant.
    if (frameCenter.isComplete() && deltaFrameSize.isComplete() && scaleFactor.isDefined())
    {
        for (size_t i = 0; i < n; i++)
        {
            if (!_lowerBound[i].isDefined())
            {
                _lowerBound[i] = frameCenter[i] - 10.0 * deltaFrameSize[i] * scaleFactor;
            }
            if (!_upperBound[i].isDefined())
            {
                _upperBound[i] = frameCenter[i] + 10.0 * deltaFrameSize[i] * scaleFactor;
            }
        }
    }

    if (!_lowerBound.isComplete())
    {
        std::string s = "LHS Lower bound needs to be completely defined. Values given: ";
        s += lowerBound.display();
        throw NOMAD::Exception(__FILE__, __LINE__, s);
    }
    if (!_upperBound.isComplete())
    {
        std::string s = "LHS Upper bound needs to be completely defined. Values given: ";
        s += upperBound.display();
        throw NOMAD::Exception(__FILE__, __LINE__, s);
    }
}


// Do the sample
// Audet & Hare Algorithm 3.9 Latin Hypercube Sampling
std::vector<NOMAD::Point> NOMAD::LHS::Sample() const
{
    std::vector<NOMAD::Point> samplepoints;

    // 0 - Initialization
    // Let Pi be a n x p matrix in which each of its n rows
    // is a random permutation of the vector (1, 2, .., p).
    //
    std::vector<std::vector<size_t>> Pi;
    for (size_t i = 0; i < _n; i++)
    {
        std::vector<size_t> v = Permutation(_p);
        Pi.push_back(v);
    }

    // 1 - Sample construction
    for (size_t j = 0; j < _p; j++)
    {
        NOMAD::Point point(_n);
        for (size_t i = 0; i < _n; i++)
        {
            NOMAD::Double r_ij = RNG::rand(0,1);
            NOMAD::Double l_i = _lowerBound[i];
            NOMAD::Double Pi_ij( (double)Pi[i][j] );
            NOMAD::Double pdouble( (double)_p );
            NOMAD::Double u_i( _upperBound[i] );

            NOMAD::Double x_ij = l_i + (Pi_ij - r_ij) / pdouble * (u_i - l_i);
            point[i] = x_ij;

        }
        samplepoints.push_back(point);
    }

    return samplepoints;
}


// Input: p
// Output: Random permutation of the vector (1, 2, .., p)
std::vector<size_t> NOMAD::LHS::Permutation(const size_t p)
{
    NOMAD::RandomPickup rp(p);

    std::vector<size_t> v;
    v.reserve(p);
    for (size_t j = 0; j < p ; j++)
    {
        v.push_back(rp.pickup()+1);
    }

    return v;
}
