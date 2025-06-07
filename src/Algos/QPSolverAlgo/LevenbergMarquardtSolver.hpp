/**
 \file   LevenbergMarquardtSolver.hpp
 \brief  Levenberg-Marquardt algorithm
 \author Tangi Migot and Ludovic Salomon
 \see    LevenbergMarquardtSolver.cpp
 */
#ifndef __NOMAD_4_5_LEVENBERG_MARQUARDT_SOLVER__
#define __NOMAD_4_5_LEVENBERG_MARQUARDT_SOLVER__

#include "../../../ext/sgtelib/src/Matrix.hpp"

#include "../../nomad_nsbegin.hpp"

enum class LMSolverStatus
{
    BOUNDS_ERROR, ///< Problem with lower bounds and upper bounds
    MATRIX_DIMENSIONS_FAILURE, ///< Problem with matrix dimensions
    MAX_ITER_REACHED, ///< Maximum number of iterations reached
    FAILURE, ///< Levenberg-Marquardt algorithm has failed
    STRICT_PT_FAILURE, ///< Computation of a strict solution has failed
    NUM_ERROR, ///< Trust-region numerical error
    PARAM_ERROR, ///< Parameter error
    TIGHT_VAR_BOUNDS, ///< Bounds on variables are too tight
    STAGNATION_ITERATES, ///< Distance between successive iterates are too low
    IMPROVED, ///< Has improved the solution
    SOLVED ///< Problem solved
};

/// Levenberg-Marquardt solver.
/// Solve:
///  min   || c(x) + s ||^2
/// (x, s)
/// s.t. lb <= x <= ub
///      s >= 0
/// NB: (x, s) is encoded as a single COLUMN vector
class LevenbergMarquardtSolver
{
public:
    LMSolverStatus solve(SGTELIB::Matrix& x,
                         SGTELIB::Matrix& XS,
                         const SGTELIB::Matrix& QPModel,
                         const SGTELIB::Matrix& lvar,
                         const SGTELIB::Matrix& uvar,
                         SGTELIB::Matrix& cX) const;

    // Parameters
    // Tolerances
    double feasibility_tol; ///< When || c(x) + s || below feasibility_tol, it is solved
    double tol; ///< The tolerance
    double tol_dist_successive_x;

    size_t max_iter; ///< The number of maximum iterations
    bool sol_be_strict; ///< Ensures the point is strictly feasible

    size_t verbose_level;

private:

    static bool checkDimensions(const SGTELIB::Matrix& x,
                                const SGTELIB::Matrix& XS,
                                const SGTELIB::Matrix& QPModel,
                                const SGTELIB::Matrix& lvar,
                                const SGTELIB::Matrix& uvar,
                                const SGTELIB::Matrix& cX);

    static bool checkBoundsCompatibilities(const SGTELIB::Matrix& lvar,
                                           const SGTELIB::Matrix& uvar,
                                           const int n);

    static bool checkBoundsTightness(const SGTELIB::Matrix& lvar,
                                     const SGTELIB::Matrix& uvar,
                                     const int n);

    bool checkStartingPointInBounds(const SGTELIB::Matrix& XS,
                                    const SGTELIB::Matrix& lvar,
                                    const SGTELIB::Matrix& uvar,
                                    const int n) const;

};

#include "../../nomad_nsend.hpp"

#endif //__NOMAD_4_5_LEVENBERG_MARQUARDT_SOLVER__
