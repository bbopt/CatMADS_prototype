/**
 \file   SimpleRNG.hpp
 \brief  Custom class for random number generator
 \author Christophe Tribes and Sebastien Le Digabel
 \date   2025-01-29
 \see    SimpleRNG.cpp
 */

#ifndef __NOMAD_4_5_SIMPLERNG__
#define __NOMAD_4_5_SIMPLERNG__

#include "../Util/defines.hpp"
#include "../Util/Exception.hpp"

using namespace std;

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

/// Class for random number generator
/**
This class is used to set a seed for the random number generator and get a random integer or a random double between two values. \n
 http://madrabbit.org/~ray/code/xorshf96.c with period 2^96-1
 */
class SimpleRNG
{
private:
    
    typedef uint32_t result_type;

    constexpr result_type min() { return 0; }
    constexpr result_type max() { return UINT32_MAX; }

public:

    SimpleRNG()
    {
        reset();
    }
    
    /// Get current seed
    /**
     \return An integer in [0,UINT32_MAX].
     */
    int getSeed()
    {
        return _x_def;
    }

    /// Set seed
    /**
     * The set seed works like a reset. The private seed used by RNG is always reset.
     \param s   The seed -- \b IN.
     */
    void setSeed(int s);

    /// Reset RNG state
    /**
     - reset the private seed to default.
     - set the rng state for the current seed
     */
    void reset()
    {
        _x = _x_def;
        _y = _y_def;
        _z = _z_def;
    }

    /// Get a random integer
    /**
     \return    An integer in the interval [0,UINT32_MAX].
     */
    result_type rand();

    /// Functor to get a random integer
    /**
     \return    An integer in the interval [0,UINT32_MAX].
     */
    result_type operator()() { return rand(); }

    /// Get a random number having a normal distribution as double
    /**
     \param a   Lower bound  -- \b IN.
     \param b   Upper bound  -- \b IN.
     \return    A double in the interval [a,b].
     */
    double rand(double a, double b)
    {
        return a + ((b - a) * rand()) / UINT32_MAX;
    }

    /// Get a random number using a normal distribution centered on 0
    /**
     * Get a random number approaching a normal distribution (N(0,Var)) as double
     *
     *
     \param Var     Variance of the target normal distribution    -- \b IN.
     \param Nsample Number of samples for averaging                -- \b IN.
     \return        A double in the interval [-sqrt(3*Var);+sqrt(3*Var)].
     */
    double normalRandMean0(double Var = 1, int Nsample = 12);

    /// Get a random number approaching a normal distribution N(Mean,Var) as double.
    /**
     A series of Nsample random numbers Xi in the interval [-sqrt(3*Var);+sqrt(3*Var)] is used -> E[Xi] = 0, Var(Xi) = var. \n
     See http://en.wikipedia.org/wiki/Central_limit_theorem

     \param Mean    Mean of the target normal distribution        -- \b IN.
     \param Var     Variance of the target normal distribution    -- \b IN.
     \return        A random number.
     */
    double normalRand(double Mean = 0, double Var = 1);


private:
    uint32_t _x_def= 123456789, _y_def= 362436069, _z_def= 521288629; ///< Initial values for the random number generator
    
    uint32_t _x, _y, _z;          ///< Current values for the random number generator

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SIMPLERNG__
