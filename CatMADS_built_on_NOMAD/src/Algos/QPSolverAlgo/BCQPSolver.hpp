#ifndef __NOMAD_BCQP_SOLVER__
#define __NOMAD_BCQP_SOLVER__

#include "../../../ext/sgtelib/src/Matrix.hpp"

#include "../../nomad_nsbegin.hpp"

enum class BCQPSolverStatus
{
    BOUNDS_ERROR, ///< Problem with lower bounds and upper bounds
    MATRIX_DIMENSIONS_FAILURE, ///< Problem with matrix dimensions
    MAX_ITER_REACHED, ///< Maximum number of iterations reached
    NUM_ERROR, ///< Conjugate gradient numerical error
    TIGHT_VAR_BOUNDS, ///< Bounds on variables are too tight
    STAGNATION_ITERATES, ///< Distance between successive iterates are too low
    SOLVED, ///< Problem solved
    UNDEFINED ///< Undefined status
};

/// BCQP solver
/// Solve the following problem
/// min Q(x) = g0 + g'x + (1/2) x'H x
///  s.t. lb <= x <= ub
class BCQPSolver
{
public:
    BCQPSolverStatus solve(SGTELIB::Matrix &x,
                           const SGTELIB::Matrix &QPModel,
                           const SGTELIB::Matrix &lb,
                           const SGTELIB::Matrix &ub) const;

    // Parameters
    double tol_dist_successive_x;

    size_t max_iter;
    size_t verbose_level;

private:
    static bool checkDimensions(const SGTELIB::Matrix& x,
                                const SGTELIB::Matrix& QPModel,
                                const SGTELIB::Matrix& lb,
                                const SGTELIB::Matrix& ub);

    static bool checkBoundsCompatibilities(const SGTELIB::Matrix& lb,
                                           const SGTELIB::Matrix& ub);

    static void projectedGradientStep(SGTELIB::Matrix& x,
                                      SGTELIB::Matrix& gradQ,
                                      std::vector<bool>& activeLowerVars,
                                      std::vector<bool>& activeUpperVars,
                                      const SGTELIB::Matrix& QPModel,
                                      const SGTELIB::Matrix& lb,
                                      const SGTELIB::Matrix& ub,
                                      const double kappa,
                                      const bool verbose);

    static double projectedArmijoLineSearch(SGTELIB::Matrix& x,
                                            SGTELIB::Matrix& gradQ,
                                            const SGTELIB::Matrix& x_start,
                                            const SGTELIB::Matrix& QPModel,
                                            const SGTELIB::Matrix& lb,
                                            const SGTELIB::Matrix& ub,
                                            const SGTELIB::Matrix& d,
                                            const double fvalue_start,
                                            const double slope,
                                            const double t_max);

    static bool conjugateGradientStep(SGTELIB::Matrix& x,
                                      const SGTELIB::Matrix& H,
                                      const SGTELIB::Matrix& g,
                                      const double xi,
                                      const bool verbose);

    // Return the maximum step size allowed and the step size chosen
    // Useful for logging
    static std::pair<double, double> lineSearchStep(SGTELIB::Matrix& x,
                                                    SGTELIB::Matrix& gradQ,
                                                    SGTELIB::Matrix& xp,
                                                    const SGTELIB::Matrix& d,
                                                    const SGTELIB::Matrix& QPModel,
                                                    const SGTELIB::Matrix& lb,
                                                    const SGTELIB::Matrix& ub,
                                                    std::vector<bool>& activeLowerVars,
                                                    std::vector<bool>& activeUpperVars,
                                                    const bool hasNegativeCurvature);

    // Return || x - P[x - grad Q(x)] ||_inf,
    // and P[X] the projection of X on [lb, ub]
    static double computeFirstOrderError(const SGTELIB::Matrix& x,
                                         const SGTELIB::Matrix& gradQ,
                                         const SGTELIB::Matrix& lb,
                                         const SGTELIB::Matrix& ub);

    static void projectOnBounds(SGTELIB::Matrix& x,
                                const SGTELIB::Matrix& lb,
                                const SGTELIB::Matrix& ub);


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_BCQP_SOLVER__