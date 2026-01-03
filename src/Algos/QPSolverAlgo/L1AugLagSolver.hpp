/**
 \file   L1AugLagSolver.hpp
 \brief  L1 Augmented Lagrangian algorithm
 \author Tangi Migot and Ludovic Salomon
 \see    L1AugLagSolver.cpp
 */
#ifndef __NOMAD_4_5_L1AUGLAG_SOLVER__
#define __NOMAD_4_5_L1AUGLAG_SOLVER__

#include "../../../ext/sgtelib/src/Matrix.hpp"

#include "../../nomad_nsbegin.hpp"

enum class L1AugLagSolverStatus
{
    BOUNDS_ERROR, ///< Problem with lower bounds and upper bounds
    MATRIX_DIMENSIONS_FAILURE, ///< Problem with matrix dimensions
    MAX_ITER_REACHED, ///< Maximum number of iterations reached
    MIN_STEPSIZE_REACHED, ///< Minimum step size reached
    NUM_ERROR, ///< Trust-region numerical error
    PARAM_ERROR, ///< Parameter error
    TIGHT_VAR_BOUNDS, ///< Bounds on variables are too tight
    STAGNATION_ITERATES, ///< Distance between successive iterates are too low
    SOLVED, ///< Problem solved
    TOO_MANY_ACTIVE_CONSTRAINTS, // At a given iterate, too many constraints (more than n) are active
    UNDEFINED ///< Undefined status
};

/// L1 AugLag solver
/// Solve the QCQP
class L1AugLagSolver
{
public:
    L1AugLagSolverStatus solve(SGTELIB::Matrix& x,
                               const SGTELIB::Matrix& QPModel,
                               const SGTELIB::Matrix& lb,
                               const SGTELIB::Matrix& ub) const;

    // Parameters
    double tol_dist_successive_x;

    size_t max_iter_outer;
    size_t max_iter_inner;

    size_t verbose_level;

private:
    L1AugLagSolverStatus solveInnerProblem(SGTELIB::Matrix& X_k,
                                           SGTELIB::Matrix& lambdaB,
                                           SGTELIB::Matrix& cons,
                                           SGTELIB::Matrix& Jk,
                                           SGTELIB::Matrix& gradLk,
                                           SGTELIB::Matrix& pseudoGradient_k,
                                           SGTELIB::Matrix& gradLd,
                                           std::vector<bool>& activeConstraints,
                                           std::vector<bool>& infeasibleConstraints,
                                           const SGTELIB::Matrix& QPModel,
                                           const SGTELIB::Matrix& lb,
                                           const SGTELIB::Matrix& ub,
                                           const SGTELIB::Matrix& lambda,
                                           const double omega,
                                           const double mu) const;

    static bool computeHorizontalStep(SGTELIB::Matrix& h_k,
                                      const SGTELIB::Matrix& X_k,
                                      const SGTELIB::Matrix& QPModel,
                                      const SGTELIB::Matrix& Jk,
                                      const std::vector<bool>& activeConstraints,
                                      const std::vector<bool>& infeasibleConstraints,
                                      const SGTELIB::Matrix& lambda,
                                      const SGTELIB::Matrix& lambdaB,
                                      const double mu,
                                      const bool considerActiveMultipliers);

    static bool computeStrengthenedStep(SGTELIB::Matrix& h_k,
                                        const SGTELIB::Matrix& QPModel,
                                        const SGTELIB::Matrix& lambda,
                                        const SGTELIB::Matrix& X_k,
                                        std::vector<bool>& activeConstraints,
                                        std::vector<bool>& infeasibleConstraints,
                                        double& inner_tol_opt,
                                        double& inner_precision_cst,
                                        const double mu);

    static bool computeDropConstraintStep(SGTELIB::Matrix& d,
                                          const SGTELIB::Matrix& JkActive,
                                          const SGTELIB::Matrix& lambdaB,
                                          const SGTELIB::Matrix& pseudoGradient,
                                          const std::vector<bool>& activeConstraints,
                                          const double mu);

    static bool computeVerticalStep(SGTELIB::Matrix &v_k,
                                    const SGTELIB::Matrix& JkActive,
                                    const SGTELIB::Matrix &QPModel,
                                    const SGTELIB::Matrix &Xcan,
                                    const std::vector<bool> &activeConstraints);

    static double piecewiseLineSearch(const SGTELIB::Matrix& X,
                                      const SGTELIB::Matrix& QPModel,
                                      const SGTELIB::Matrix& d,
                                      const SGTELIB::Matrix& lb,
                                      const SGTELIB::Matrix& ub,
                                      const std::vector<bool>& activeConstraints,
                                      const std::vector<bool>& infeasibleConstraints,
                                      const SGTELIB::Matrix& lambda,
                                      const double mu,
                                      const double small_gamma, // = 1E-20
                                      const double gamma_update, // = 1.5
                                      const double delta /* = 1E-4 // Pk < (P0 - delta) */ );

    static double quadraticInterpolationSearch(const SGTELIB::Matrix& X,
                                               const SGTELIB::Matrix& QPModel,
                                               const SGTELIB::Matrix& d,
                                               const SGTELIB::Matrix& lb,
                                               const SGTELIB::Matrix& ub,
                                               const SGTELIB::Matrix& lambda,
                                               const double mu);

    static bool checkDimensions(const SGTELIB::Matrix& x,
                                const SGTELIB::Matrix& QPModel,
                                const SGTELIB::Matrix& lb,
                                const SGTELIB::Matrix& ub);

    static bool checkBoundsCompatibilities(const SGTELIB::Matrix& lb,
                                           const SGTELIB::Matrix& ub);

    static void projectOnBounds(SGTELIB::Matrix& x,
                                const SGTELIB::Matrix& lb,
                                const SGTELIB::Matrix& ub);

    // Return || x - P[x - grad L(x)] ||_inf, where L is the lagrangian at x
    // and P[X] the projection of X on [lb, ub]
    static double computeFirstOrderError(const SGTELIB::Matrix& x,
                                         const SGTELIB::Matrix& gradLk,
                                         const SGTELIB::Matrix& lb,
                                         const SGTELIB::Matrix& ub);

    static void computeActiveConstraints(std::vector<bool>& activeConstraints,
                                         const SGTELIB::Matrix& cons,
                                         const double inner_tol);

    static void computeInfeasibleConstraints(std::vector<bool>& infeasibleConstraints,
                                             const SGTELIB::Matrix& cons,
                                             const double inner_tol);

    static void computeMultipliersInfeasibleConstraints(SGTELIB::Matrix& lambda,
                                                        const SGTELIB::Matrix& gradL,
                                                        const SGTELIB::Matrix& J,
                                                        const std::vector<bool>& activeConstraints,
                                                        const std::vector<bool>& infeasibleConstraints,
                                                        const double mu);

    // Compute grad D(x, lambda) = grad L(x, lambda) + (1/mu) sum_V grad cj(x)
    // where V is the set of infeasible constraints
    static void computePseudoGradient(SGTELIB::Matrix& gradD,
                                      const SGTELIB::Matrix& gradL,
                                      const SGTELIB::Matrix& J,
                                      const std::vector<bool>& infeasibleConstraints,
                                      const double mu);

    // Compute grad LD(x, lambda) = grad D(x, lambda) - lambdaB' cA(x)
    // where A is the set of active constraints
    static void computeGradLd(SGTELIB::Matrix& gradLd,
                              const SGTELIB::Matrix& gradL,
                              const SGTELIB::Matrix& J,
                              const SGTELIB::Matrix& lambdaB,
                              const std::vector<bool>& activeConstraints,
                              const std::vector<bool>& infeasibleConstraints,
                              const double mu);

    static SGTELIB::Matrix extractActiveJacobianCons(const SGTELIB::Matrix& J,
                                                     const std::vector<bool>& activeConstraints);

    static double computeL1AugmentedLagrangianVal(const SGTELIB::Matrix& QPModel,
                                                  const SGTELIB::Matrix& x,
                                                  const SGTELIB::Matrix& lambda,
                                                  const double mu);
};

#include "../../nomad_nsend.hpp"

#endif //__NOMAD_4_5_L1AUGLAG_SOLVER__
