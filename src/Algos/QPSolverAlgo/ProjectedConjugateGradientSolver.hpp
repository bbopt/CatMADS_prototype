/**
 \file   ProjectedConjugateGradientSolver.hpp
 \brief  Projected Conjugate Gradient algorithm
 \author Tangi Migot and Ludovic Salomon
 \see    ProjectedConjugateGradientSolver.cpp
 */
#ifndef __NOMAD_4_5_PROJECTED_CONJUGATE_GRADIENT_SOLVER__
#define __NOMAD_4_5_PROJECTED_CONJUGATE_GRADIENT_SOLVER__

#include "../../../ext/sgtelib/src/Matrix.hpp"

#include "../../nomad_nsbegin.hpp"

enum class ProjectedConjugateGradientSolverStatus
{
    MATRIX_DIMENSIONS_FAILURE, ///< Problem with matrix dimensions
    FACTORIZATION_FAILURE, ///< Factorization for solving linear systems has failed
    QUAD_ROOTS_ERROR, ///< Resolution of quadratic roots subproblem has failed
    TR_NUM_ERROR, ///< Trust-region numerical error
    TR_PARAM_ERROR, ///< Trust-region parameter error
    MAX_ITER_REACHED, ///< Maximum number of iterations reached
    BOUNDARY_REACHED, ///< The algorithm stops because boundary of the trust-region is reached
    NEGATIVE_CURVATURE, ///< The G matrix is not symmetric definite-positive
    NO_INIT_SOLUTION, ///< No initial solution satisfying constraints
    SOLVED ///< Solved: usually when minimum tolerance reached.
};

/// Projected conjugate gradient solver
/// Solve:
/// min 1/2 x' G x + c' x
/// s.t. A x = b
///      |x| <= delta
class ProjectedConjugateGradientSolver
{
public:
    static ProjectedConjugateGradientSolverStatus solve(SGTELIB::Matrix& x,
                                                        const SGTELIB::Matrix& G,
                                                        const SGTELIB::Matrix& c,
                                                        const SGTELIB::Matrix& A,
                                                        const SGTELIB::Matrix& b,
                                                        const double delta,
                                                        const bool verbose = false);
};

#include "../../nomad_nsend.hpp"

#endif //__NOMAD_4_5_PROJECTED_CONJUGATE_GRADIENT_SOLVER__
