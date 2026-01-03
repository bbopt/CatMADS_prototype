#ifndef __NOMAD_4_5_MATH_UTILS__
#define __NOMAD_4_5_MATH_UTILS__

#include "../Util/defines.hpp"

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

/// Mathematics utility functions.
/// Contains some mathematics utility functions that do not use the type NOMAD::Double.

/// Find the real roots of the quadratic
/// q(x) = q2 x^2 + q1 x + q0
///
/// Care is taken to avoid numerical cancellation. It is adapted from the package Krylov.jl.
/**
 \param q2 order 2 coefficient value -- \b IN.
 \param q1 order 1 coefficient value -- \b IN.
 \param q0 order 0 coefficient value -- \b IN.
 \param r1 first real root value -- \b OUT.
 \param r2 second real root value -- \b OUT.
 \return \c true if real roots exist

 NB: if only one root value exists, r1 and r2 will be equal.
 */
DLL_UTIL_API bool roots_quadratic(const double q2, const double q1, const double q0,
                                  double& r1, double& r2);

#include "../nomad_nsend.hpp"
#endif // __NOMAD_4_5_MATH_UTILS__
