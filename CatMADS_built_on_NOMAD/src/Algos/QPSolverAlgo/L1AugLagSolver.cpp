/**
 \file   L1AugLagSolver.cpp
 \brief  L1 Augmented Lagrangian algorithm: implementation
 \author Tangi Migot and Ludovic Salomon
 \see    L1AugLagSolver.hpp
 */
#include "L1AugLagSolver.hpp"

#include "../../Nomad/nomad.hpp"
#include "QPModelUtils.hpp"

NOMAD::L1AugLagSolverStatus NOMAD::L1AugLagSolver::solve(SGTELIB::Matrix& x,
                                                         const SGTELIB::Matrix& QPModel,
                                                         const SGTELIB::Matrix& lb,
                                                         const SGTELIB::Matrix& ub) const
{
    if (!checkDimensions(x, QPModel, lb, ub))
    {
        return NOMAD::L1AugLagSolverStatus::MATRIX_DIMENSIONS_FAILURE;
    }

    if (!checkBoundsCompatibilities(lb, ub))
    {
        return NOMAD::L1AugLagSolverStatus::BOUNDS_ERROR;
    }

    const int n = x.get_nb_rows();

    // Particular case : when the gap between lb and ub is too small, we immediately leave the procedure.
    // NB: A scaling could be applied to prevent this behavior. But at extremely low tolerance, this is not
    // really useful. The solver could have difficulty to move in the decision space.
    for (int i = 0; i < n; ++i)
    {
        if (std::abs(lb.get(i, 0) - ub.get(i, 0)) <= 1e-8)
        {
            return NOMAD::L1AugLagSolverStatus::TIGHT_VAR_BOUNDS;
        }
    }

    // Stopping tolerances
    constexpr double tol_opt = 1e-5;
    constexpr double tol_feas = 1e-5;
    //constexpr double rtol = 1e-7;
    // constexpr double atol = 1e-7;

    // Compute stopping tolerance
    //SGTELIB::Matrix gradientLag_k("gradientLag_k", _n, 1);
    //getModelGrad(&gradientLag_k, X_k);
    //double ng = gradientLag_k.norm();
    //const double ng0 = ng;
    //const double tol = atol + ng0 * rtol;
    //verbose && std::cout << "Start solveL1AugLag with tol = " << tol << std::endl;

    // Compute initial point
    projectOnBounds(x, lb, ub);

    const int nbCons = QPModel.get_nb_rows() - 1;
    // Allocation of vector for outer loop
    SGTELIB::Matrix cons("cons", nbCons, 1); // c(x)
    SGTELIB::Matrix gradLk("gradLk", n, 1); // L(x, lambda)

    // Allocate vectors for inner loop
    SGTELIB::Matrix Jk("Jk", nbCons, n); // Jacobian of the constraints
    SGTELIB::Matrix gradLd("gradLd", n, 1);
    SGTELIB::Matrix lambdaB("lambdaB", nbCons, 1); // lambda_bar
    SGTELIB::Matrix pseudoGradient_k("pseudoGradient_k", n, 1); // Pseudo-gradient
    SGTELIB::Matrix multiplier_k("multiplier_k", nbCons, 1); // Multipliers to compute pseudo-gradient
    SGTELIB::Matrix activeMultiplier_k;

    SGTELIB::Matrix X_k("X_k", n, 1);
    X_k = x;
    SGTELIB::Matrix X_km1("X_km1", n, 1);

    // Outer loop augmented Lagrangian multipliers
    SGTELIB::Matrix lambda_l("lambda_l", nbCons, 1);
    lambda_l.fill(0.0);
    double mu_l = 0.1;
    double eta_l = 1.0;
    double omega_l = 1.0;

    // Allocate memory for:
    // * the set of active constraints, i.e., j such that |cj(x)| <= epsilon
    // * the set of infeasible constraints, i.e., j such that cj(x) > epsilon
    // * the set of strictly feasible constraints, i.e., j such that cj(x) < -epsilon
    // where epsilon > 0 is a tolerance parameter
    std::vector<bool> activeConstraints(nbCons);
    std::vector<bool> infeasibleConstraints(nbCons);
    std::vector<bool> feasibleConstraints(nbCons);

    // NB: for the outer loop, we do not consider the distance between successive
    // iterates. For example, if the current iterate has a stopping criterion
    // value far below the threshold omega, the algorithm will update the
    // lagrangian multipliers, even if the iterate remains identical.
    // Even though the inner sub-problem is not solved at optimality
    // in next iterates, we could hope that the algorithm will keep on.
    // This is only kept for logging.
    double distXOuterLoop = NOMAD::INF;

    // Stopping criterion parameter
    size_t successive_failure = 0;
    size_t successive_acceptable = 0;
    constexpr size_t successive_failure_before_update = 2;
    constexpr size_t max_successive_failure = 5;

    // Set tolerances
    // a- Compute || x0 - P[x - grad f(x0)] ||_inf, where P[X] is the projection of x on [lb, ub]
    SGTELIB::Matrix gradL("gradL", n, 1);
    QPModelUtils::getModelObjGrad(gradL, QPModel, x);
    const double nprojFx0 = computeFirstOrderError(x, gradL, lb, ub);

    // b- Compute || max(c(x0), 0) ||_inf
    const double ncx0 = [](const SGTELIB::Matrix& QPModel, const SGTELIB::Matrix& x)
    {
        const int nbCons = QPModel.get_nb_rows() - 1;
        double ncx0 = 0.0;
        for (int j = 0; j < nbCons; ++j)
        {
            ncx0 = std::max(ncx0, QPModelUtils::getModelCons(QPModel, j, x));
        }
        return ncx0;
    }(QPModel, x);

    auto areConstraintsBelowTol = [](const SGTELIB::Matrix &cons, const double tol)
    {
        const int nbCons = cons.get_nb_rows();
        for (int i = 0; i < nbCons; ++i)
        {
            if (cons.get(i, 0) > tol)
                return false;
        }
        return true;
    };

    const bool verbose = verbose_level > 0;
    if (verbose)
    {
        std::printf("\nL1 augmented lagrangian algorithm\n");
        std::printf("Number of total variables: %d\n", n);
        std::printf("Number of inequality constraints: %d\n", nbCons);
        std::printf("Stopping criterion tolerance for optimality: %e\n", tol_opt * std::max(1.0, nprojFx0));
        std::printf("Stopping criterion tolerance for feasibility: %e\n", tol_feas * std::max(1.0, ncx0));
        std::printf("Maximum number of iterations allowed for outer loop: %zu\n", max_iter_outer);
        std::printf("Maximum number of iterations allowed for inner loop: %zu\n\n", max_iter_inner);

        std::printf("%12s %15s %22s %28s %16s %18s %8s %10s %8s %12s %12s %20s %12s\n",
                    "iter (outer)", "f(x)", "|| max(c(x), 0) ||_inf", "|| x - P[x-grad L(x)] ||_inf",
                    "|| lambda ||_inf", "|| lambda_B ||_inf",
                    "|Active|", "|Infeasible|",
                    "mu", "omega", "eta", "|| x - xp ||_inf", "status inner");
        constexpr int maxLineWidth = 207;
        for (int i = 0; i < maxLineWidth; ++i)
        {
            std::printf("-");
        }
        std::printf("\n");
    }

    // Outer loop
    L1AugLagSolverStatus status = L1AugLagSolverStatus::MAX_ITER_REACHED;
    L1AugLagSolverStatus innerSolverStatus = L1AugLagSolverStatus::UNDEFINED;
    for (size_t iter_outer = 0; iter_outer < max_iter_outer; ++iter_outer)
    {
        QPModelUtils::getModelCons(cons, QPModel, X_k);
        QPModelUtils::getModelLagrangianGrad(gradLk, QPModel, X_k, lambda_l);

        const double ngproj = computeFirstOrderError(X_k, gradLk, lb, ub);
        if (verbose)
        {
            const double objVal = QPModelUtils::getModelObj(QPModel, X_k);
            double normCxMax0 = 0.0;
            for (int i = 0; i < nbCons; ++i)
            {
                normCxMax0 = std::max(cons.get(i, 0), normCxMax0);
            }

            const std::string statusLog = [](const L1AugLagSolverStatus status) -> std::string
            {
                if (status == L1AugLagSolverStatus::UNDEFINED)
                {
                    return "-";
                }
                if (status == L1AugLagSolverStatus::MAX_ITER_REACHED)
                {
                    return "Max iteration reached";
                }
                if (status == L1AugLagSolverStatus::MIN_STEPSIZE_REACHED)
                {
                    return "Min step size reached";
                }
                if (status == L1AugLagSolverStatus::SOLVED)
                {
                    return "Solved";
                }
                if (status == L1AugLagSolverStatus::STAGNATION_ITERATES)
                {
                    return "Stagnation iterates";
                }
                if (status == L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS)
                {
                    return "Many active constraints";
                }

                return "error";
            }(innerSolverStatus);

            if (iter_outer == 0)
            {
                std::printf(" %-12zu %14e %16e %25e %23e %16e %10s %10s %16e %10e %10e %12e %12s\n",
                            iter_outer, objVal, normCxMax0, ngproj,
                            lambda_l.norm(), lambdaB.norm(),
                            "-", "-", mu_l, omega_l, eta_l,
                            distXOuterLoop, statusLog.c_str());
            } else
            {
                const int nbActive = (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);
                const int nbInfeasible = (int) std::count(infeasibleConstraints.begin(), infeasibleConstraints.end(), true);
                std::printf(" %-12zu %14e %16e %25e %23e %16e %10d %10d %16e %10e %10e %12e %12s\n",
                            iter_outer, objVal, normCxMax0, ngproj,
                            lambda_l.norm(), lambdaB.norm(),
                            nbActive, nbInfeasible, mu_l, omega_l, eta_l,
                            distXOuterLoop, statusLog.c_str());
            }
        }

        // Check stopping criterion
        if (ngproj <= tol_opt * std::max(1.0, nprojFx0) && areConstraintsBelowTol(cons, tol_feas * std::max(1.0, ncx0)))
        {
            status = L1AugLagSolverStatus::SOLVED;
            x = X_k;
            break;
        }

        // Check stagnation
        if (mu_l <= tol_opt / 10.0 ||
            successive_failure >= max_successive_failure)
        {
            x = X_k;
            status = L1AugLagSolverStatus::STAGNATION_ITERATES;
            break;
        }

        // Inner loop initialization
        // Save current iterate
        X_km1 = X_k;

        QPModelUtils::getModelJacobianCons(Jk, QPModel, X_k);

        // Solve subproblem
        innerSolverStatus = solveInnerProblem(X_k, lambdaB, cons, Jk, gradLk, pseudoGradient_k, gradLd,
                                              activeConstraints, infeasibleConstraints,
                                              QPModel, lb, ub, lambda_l, omega_l, mu_l);

        const bool innerSolverSuccess = innerSolverStatus == L1AugLagSolverStatus::SOLVED ||
                                        innerSolverStatus == L1AugLagSolverStatus::STAGNATION_ITERATES ||
                                        innerSolverStatus == L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS ||
                                        innerSolverStatus == L1AugLagSolverStatus::MAX_ITER_REACHED ||
                                        innerSolverStatus == L1AugLagSolverStatus::MIN_STEPSIZE_REACHED;
        if (!innerSolverSuccess)
        {
            // Something has been wrong: exit
            x = X_k;
            status = innerSolverStatus;
            break;
        }

        // Check the new iterate is w-critical
        computeGradLd(gradLd, gradLk, Jk, lambdaB, activeConstraints, infeasibleConstraints, mu_l);
        if (computeFirstOrderError(X_k, gradLd, lb, ub) <= tol_opt * std::max(1.0, nprojFx0) &&
            areConstraintsBelowTol(cons, tol_feas * std::max(1.0, ncx0)))
        {
            x = X_k;
            status = L1AugLagSolverStatus::SOLVED;
            break;
        }

        // Special case: when too many constraints are active, stop the procedure, the algorithm
        // is stuck.
        if (innerSolverStatus == L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS)
        {
            x = X_k;
            status = L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS;
            break;
        }

        // Update Lagrange multipliers
        if (areConstraintsBelowTol(cons, eta_l))
        {
            for (int j = 0; j < nbCons; ++j)
            {
                const double lambda_j = lambda_l.get(j, 0);
                if (activeConstraints[j])
                {
                    const double lambdaB_j = lambdaB.get(j, 0);
                    lambda_l.set(j, 0, lambda_j + lambdaB_j);
                    continue;
                }
                if (infeasibleConstraints[j])
                {
                    lambda_l.set(j, 0, lambda_j - 1.0 / mu_l);
                }
                // Otherwise, no update of lambda
            }
            eta_l = eta_l * pow(mu_l, 0.9);

            // Update omega
            if (innerSolverStatus == L1AugLagSolverStatus::SOLVED)
            {
                omega_l = std::max(omega_l * mu_l, 1e-7);
                successive_acceptable = 0;
                successive_failure = 0;
            }
            else if (successive_acceptable >= successive_failure_before_update)
            {
                // Decrease the tolerance slower
                omega_l = std::max(omega_l * std::sqrt(mu_l), 1e-7);
                successive_acceptable += 1;
                successive_failure += 1;
            }
            else
            {
                // Restart from this point
                successive_acceptable += 1;
            }
        }
        else
        {
            // No update of Lagrange multipliers
            // Update mu
            if (innerSolverStatus == L1AugLagSolverStatus::SOLVED)
            {
                mu_l = mu_l / 10.0;
                successive_failure = 0;
            }
            else if (successive_acceptable >= successive_failure_before_update)
            {
                // The subproblem has not been solved at optimality, but we still want to keep on
                // iterating. If mu decreases too fast, the algorithm can fail quick.
                mu_l = mu_l / std::sqrt(10.0);
                successive_failure += 1;
            }
            else
            {
                successive_acceptable += 1; // Try rerun from new point
            }

            // Update eta and omega
            eta_l = pow(mu_l, 0.1) / 10.0;
            omega_l = mu_l;
        }

        distXOuterLoop = SGTELIB::Matrix::distNorm2(X_k, X_km1);
    }

    if (verbose)
    {
        std::printf("\nStatus: ");
        std::printf("f(x*) = %e\n", QPModelUtils::getModelObj(QPModel, x));
        QPModelUtils::getModelCons(cons, QPModel, x);
        double normCxMax0 = 0;
        for (int i = 0; i < nbCons; ++i)
        {
            normCxMax0 = std::max(cons.get(i, 0), normCxMax0);
        }
        std::printf("|| max (c(x), 0) ||_inf = %e\n", normCxMax0);

        if (status == L1AugLagSolverStatus::SOLVED)
        {
            std::printf("Has reached the minimum tolerance\n");
            // std::printf("|| x - P[x - grad L(x)] ||_inf = %e <= max(1.0, || x - P[x - grad f(x)] ||) * atol_opt = %e and\n",
            //             err_metric.projlagGradNorm, std::max(1.0, err_metric.projObjGrad) * atol_opt);
            // std::printf("|| max(0, c(x)) ||_inf = %e <= max(1.0, || max(0, c(x0)) ||_inf) * atol_feas = %e\n",
            //             err_metric.cxNorm, std::max(1.0, err_metric.cxInitNorm) * atol_feas);
        }
        else if (status == L1AugLagSolverStatus::STAGNATION_ITERATES)
        {
            std::printf("Outer steps have stagnated:\n");
            std::printf("|| x - xp || = %e <= %e or\n", distXOuterLoop, tol_dist_successive_x);
            std::printf("mu = %e <= tol_opt / mu_decrease = %e or\n", mu_l, tol_opt / 10.0);
            std::printf("The inner solver has not made sufficient progress %zu times, superior to the following threshold = %zu\n",
                        successive_failure, max_successive_failure);
        }
        else if (status == L1AugLagSolverStatus::MAX_ITER_REACHED)
        {
            std::printf("The maximum number of outer iterations has been reached\n");
        }
        else if (status == L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS)
        {
            std::printf("Too many constraints are active for the minimum tolerance allowed: stop\n");
        }
        else
        {
            std::printf("Unknown stopping criterion\n");
        }
    }

    return status;
}

NOMAD::L1AugLagSolverStatus NOMAD::L1AugLagSolver::solveInnerProblem(SGTELIB::Matrix& X_k,
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
                                                                     const double mu) const
{
    // Utility lambda
    auto checkLambdaBCriticality = [](const SGTELIB::Matrix &lambdaB, const double mu)
    {
        // NB: one has to be careful with the sign of lambdaB.
        // The considered problem is:
        // min f(x)
        // s.t c(x) <= 0
        // At optimality, the Lagrange multipliers corresponding to the constraints are <= 0.
        //
        // The work of T.F. Colemann and A.R. Conn considers
        // min f(x)
        // s.t c(x) >= 0
        // and derive stopping criterion for Lagrange multipliers based on the fact that they
        // are positive at optimality.
        //
        // Hence, in our case, lambdaB is critical if lambdaB in [-1/mu, 0]
        //
        // The description of the algorithm in Fig 4.1
        //
        // Modeles quadratiques et decomposition parallele pour
        // l'optimisation sans derivees
        // by N. Amaioua (2018)
        //
        // does not consider this fact.
        //
        // NB2: The original implementation of N. Amaioua adds a minus sign to the Jacobian of active constraints
        // before decomposing the matrix and compute lambdaB, such that lambdaB remains in [0, 1/mu].
        // In summary, there are some mistakes in the description of the algorithm, but the
        // original implementation remains correct.
        const int nbCons = lambdaB.get_nb_rows();
        for (int j = 0; j < nbCons; ++j)
        {
            const double lambdaBi = lambdaB.get(j, 0);
            if (lambdaBi > 0)
            {
                return false;
            }
            if (lambdaBi < -1.0 / mu)
            {
                return false;
            }
        }
        return true;
    };

    auto areConstraintsBelowTol = [](const SGTELIB::Matrix &cons, const double tol)
    {
        const int nbCons = cons.get_nb_rows();
        for (int i = 0; i < nbCons; ++i)
        {
            if (cons.get(i, 0) > tol)
                return false;
        }
        return true;
    };

    const int n = X_k.get_nb_rows();

    // Allocate memory for the following vectors
    SGTELIB::Matrix JkActive; // Active jacobian constraints
    SGTELIB::Matrix h_k("h_k", n, 1); // Horizontal step
    SGTELIB::Matrix v_k("v_k", n, 1); // Vertical step
    SGTELIB::Matrix d_k("d_k", n, 1); // Drop constraint step
    SGTELIB::Matrix ZtransposePseudoGrad_k;
    SGTELIB::Matrix X_kp1("X_kp1", n, 1); // Previous iterate
    SGTELIB::Matrix Xcan("Xcan", n, 1);

    // Initialize tolerance for
    // tolerance related to the computation of the lambda_bar multipliers inner loop
    double inner_tol_opt = 1.0;
    // tolerance related to the computation of active, infeasible and violated constraints
    double inner_precision_cst = 1.0;

    // Minimum tolerances allowed
    constexpr double min_inner_tol_opt = 1e-5;
    constexpr double min_inner_precision_cst = 1e-5;

    const bool verbose = verbose_level > 1;
    if (verbose)
    {
        std::printf("\nL1 augmented lagrangian inner loop algorithm\n");
        std::printf("Number of variables: %d\n", n);
        std::printf("Number of inequality constraints: %d\n", lambda.get_nb_rows());
        std::printf("|| lambda ||_inf = %e\n", lambda.norm_inf());
        std::printf("Maximum number of iterations allowed for inner loop: %zu\n", max_iter_inner);
        std::printf("mu parameter: %e\n", mu);
        std::printf("stopping tolerance parameter (omega): %e\n", omega);
        std::printf("min_inner_tol_opt parameter: %e\n", min_inner_tol_opt);
        std::printf("min_inner_precision_cst parameter: %e\n\n", min_inner_precision_cst);

        std::printf("%12s %12s %24s %26s %19s %10s %13s %13s %14s %14s %12s %13s %17s %10s %16s\n",
                    "iter (inner)", "L(x)", "|| max(0, c(x)) ||_inf", "|| x - P[x-grad Ld(x)] ||",
                    "|| lambda_B ||_inf", "|| h ||", "|| v ||", "|| d ||", "|| s ||", "step size",
                    "|Active|", "|Infeasible|",
                    "|| x - xp ||_inf", "tol opt", "precision cst");
        constexpr int maxLineWidth = 240;
        for (int i = 0; i < maxLineWidth; ++i)
        {
            std::printf("-");
        }
        std::printf("\n");
    }

    double distXInnerLoop = NOMAD::INF;

    bool computeActiveAndInfeasibleSets = true;
    bool considerMultipliersEstimates = true;

    // For logging
    bool dropConstraintStepExecuted = false;
    bool horizontalStepExecuted = false;
    bool verticalStepExecuted = false;
    bool strenghtenedStepExecuted = false;
    double stepSizeLogVal = 1.0;

    double nLdproj = 0;
    double projPGradNorm = 0;

    L1AugLagSolverStatus status = L1AugLagSolverStatus::MAX_ITER_REACHED;
    for (size_t iter_inner = 0; iter_inner < max_iter_inner; ++iter_inner)
    {
        // Compute set of active and infeasible constraints
        if (computeActiveAndInfeasibleSets)
        {
            computeActiveConstraints(activeConstraints, cons, inner_precision_cst);
            computeInfeasibleConstraints(infeasibleConstraints, cons, inner_precision_cst);

            // If the number of active constraints is superior to the number of variables,
            // often the nullspace of the Jacobian of active constraints will have dimension 0.
            // To avoid this case, try to decrease the inner_precision_cst threshold
            int nbActiveConstraints = (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);
            while (nbActiveConstraints >= n)
            {
                inner_precision_cst /= 2.0;
                inner_tol_opt /= 2.0;
                inner_tol_opt = std::max(inner_tol_opt, min_inner_tol_opt);
                if (inner_precision_cst < min_inner_precision_cst)
                    break;

                computeActiveConstraints(activeConstraints, cons, inner_precision_cst);
                computeInfeasibleConstraints(infeasibleConstraints, cons, inner_precision_cst);
                nbActiveConstraints = (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);
            }
        }

        // It is possible that the algorithm has reached the minimum tolerance for the constraints
        // and there are no more active constraints.
        // In this case, reset inner_precision_cst to the minimum tolerance allowed
        inner_precision_cst = std::max(inner_precision_cst, min_inner_precision_cst);

        // Compute lambda_bar
        computeMultipliersInfeasibleConstraints(lambdaB, gradLk, Jk, activeConstraints, infeasibleConstraints, mu);

        // Compute Ld(x, lambda, mu, lambda_ba)
        computeGradLd(gradLd, gradLk, Jk, lambdaB, activeConstraints, infeasibleConstraints, mu);

        // Check stopping criterion
        // Is the current iterate w-critical ?
        nLdproj = computeFirstOrderError(X_k, gradLd, lb, ub);

        if (verbose)
        {
            const double lagVal = QPModelUtils::getModelLagrangian(QPModel, X_k, lambda);
            double normMaxCx0 = 0;
            for (int j = 0; j < cons.get_nb_rows(); ++j)
            {
                normMaxCx0 = std::max(cons.get(j, 0), normMaxCx0);
            }

            std::printf(" %-12zu", iter_inner);
            std::printf(" %+12e", lagVal);
            std::printf(" %18e", normMaxCx0);
            std::printf(" %24e", nLdproj);
            std::printf(" %22e", lambdaB.norm());
            if (horizontalStepExecuted)
            {
                std::printf(" %16e", h_k.norm());
            }
            else
            {
                std::printf(" %16s", "-     ");
            }
            if (verticalStepExecuted)
            {
                std::printf(" %14e", v_k.norm());
            }
            else
            {
                std::printf(" %14s", "-     ");
            }
            if (dropConstraintStepExecuted)
            {
                std::printf(" %14e", d_k.norm());
            }
            else
            {
                std::printf(" %14s", "-       ");
            }
            if (strenghtenedStepExecuted)
            {
                std::printf(" %14e", h_k.norm());
            }
            else
            {
                std::printf(" %14s", "-       ");
            }
            std::printf(" %10e", stepSizeLogVal);
            const int nbActiveConstraints = (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);
            std::printf(" %6d", nbActiveConstraints);
            const int nbInfeasibleConstraints = (int) std::count(infeasibleConstraints.begin(), infeasibleConstraints.end(), true);
            std::printf(" %10d", nbInfeasibleConstraints);
            std::printf(" %20e", distXInnerLoop);
            std::printf(" %16e", inner_tol_opt);
            std::printf(" %11e\n", inner_precision_cst);
        }

        if (nLdproj <= omega && checkLambdaBCriticality(lambdaB, mu))
        {
            status = NOMAD::L1AugLagSolverStatus::SOLVED;
            break;
        }

        if (stepSizeLogVal <= 1e-15)
        {
            status = NOMAD::L1AugLagSolverStatus::MIN_STEPSIZE_REACHED;
            break;
        }

        if (distXInnerLoop <= tol_dist_successive_x)
        {
            status = NOMAD::L1AugLagSolverStatus::STAGNATION_ITERATES;
            break;
        }

        const int nbActiveConstraints = (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);
        if (nbActiveConstraints >= n)
        {
            status = NOMAD::L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS;
            break;
        }

        computePseudoGradient(pseudoGradient_k, gradLk, Jk, infeasibleConstraints, mu);
        if (nbActiveConstraints > 0)
        {
            JkActive = extractActiveJacobianCons(Jk, activeConstraints);
            auto Zk = JkActive.null_space();
            ZtransposePseudoGrad_k = SGTELIB::Matrix::product(Zk.transpose(), pseudoGradient_k);
        }
        else
        {
            ZtransposePseudoGrad_k = pseudoGradient_k;
        }

        // Check the second stopping criterion for the inner iteration
        // NB: In the reference
        //
        // Nonlinear programming via an exact penalty function: Global analysis.
        // by T.F. Coleman and A.R. Conn (1982),
        // Mathematical Programming, 24(1), 137-161,
        //
        // the stopping criterion does not take into account bound constraints.
        // As the norm ||  Z' pseudoGrad || quantifies first-order optimality, when there are no bound constraints,
        // one can extend it to the bound-constrained case by using || x - P[x - Z' pseudoGrad] ||,
        // where P[X] is the projection of X on [lb, ub]; the smallest this value is, the more impact the multipliers
        // for the active constraints are.
        // TODO: one could also call this function
        // Compute || pseudoGradient_k - activeJacobian_k t active_multiplier_k ||.
        // At optimality, we should have pseudoGradient_k (approx) = activeJacobian_k t active_multiplier_k
        // which can be used as a stopping criterion.
        // double dual_norm = compute_dual_residual(pseudoGradient_k, activeJacobian_k, active_multiplier_k);
        // dual_norm = check_inner_success(X_k, Jacobian_k, multiplier_k, lambda_l, mu_l, active, infeasible);
        // innerSuccess = (dual_norm < tol) && isFeasible(cons, tol);
        projPGradNorm = computeFirstOrderError(X_k, pseudoGradient_k, lb, ub);
        if (projPGradNorm < min_inner_tol_opt &&
            checkLambdaBCriticality(lambdaB, mu) && areConstraintsBelowTol(cons, min_inner_precision_cst))
        {
            status = NOMAD::L1AugLagSolverStatus::SOLVED;
            break;
        }

        // Save current iterate
        X_kp1 = X_k;

        // Initialize these flags
        dropConstraintStepExecuted = false;
        horizontalStepExecuted = false;
        verticalStepExecuted = false;
        strenghtenedStepExecuted = false;

        if (nbActiveConstraints == 0 || ((projPGradNorm > inner_tol_opt) && considerMultipliersEstimates))
        {
            // Execute a line search along the horizontal step only.
            // NB: The active multipliers are not taken into account here.
            const bool horizontalSuccess = computeHorizontalStep(h_k, X_k, QPModel, Jk,
                                                                 activeConstraints, infeasibleConstraints,
                                                                 lambda, lambdaB, mu, false);
            if (!horizontalSuccess)
            {
                return L1AugLagSolverStatus::NUM_ERROR;
            }

            const double stepSize = piecewiseLineSearch(X_k, QPModel, h_k, lb, ub, activeConstraints, infeasibleConstraints,
                                                        lambda, mu, 1e-15, 1.5, 1e-7);

            // Update the current iterate
            for (int i = 0; i < n; ++i)
            {
                const double xki = X_k.get(i, 0);
                const double hki = h_k.get(i, 0);
                X_k.set(i, 0, xki + stepSize * hki);
            }
            projectOnBounds(X_k, lb, ub);

            // Recompute parameters for detecting stopping criterion
            QPModelUtils::getModelLagrangianGrad(gradLk, QPModel, X_k, lambda);
            QPModelUtils::getModelCons(cons, QPModel, X_k);
            QPModelUtils::getModelJacobianCons(Jk, QPModel, X_k);
            distXInnerLoop = SGTELIB::Matrix::distNorm2(X_k, X_kp1);
            computeActiveAndInfeasibleSets = true;
            considerMultipliersEstimates = true;

            horizontalStepExecuted = true;
            stepSizeLogVal = stepSize;

            continue;
        }

        // Try to compute a drop constraint direction
        const bool dropConstraintSuccess = computeDropConstraintStep(d_k, JkActive, lambdaB,
                                                                     pseudoGradient_k, activeConstraints, mu);
        if (dropConstraintSuccess)
        {
            // Apply a line search along the direction d_k
            const double stepSize = piecewiseLineSearch(X_k, QPModel, d_k, lb, ub, activeConstraints, infeasibleConstraints,
                                                        lambda, mu, 1e-15, 1.5, 1e-7);

            // Update the current iterate
            for (int i = 0; i < n; ++i)
            {
                const double xki = X_k.get(i, 0);
                const double dki = d_k.get(i, 0);
                X_k.set(i, 0, xki + stepSize * dki);
            }
            projectOnBounds(X_k, lb, ub);

            // Recompute parameters for detecting stopping criterion
            QPModelUtils::getModelLagrangianGrad(gradLk, QPModel, X_k, lambda);
            QPModelUtils::getModelCons(cons, QPModel, X_k);
            QPModelUtils::getModelJacobianCons(Jk, QPModel, X_k);
            distXInnerLoop = SGTELIB::Matrix::distNorm2(X_k, X_kp1);
            computeActiveAndInfeasibleSets = true;
            considerMultipliersEstimates = true;

            dropConstraintStepExecuted = true;
            stepSizeLogVal = stepSize;

            continue;
        }

        if (!checkLambdaBCriticality(lambdaB, mu))
        {
            // Exploring along d_k does not reach sufficient decrease
            // compute a strengthened step
            const bool strengthenedSuccess = computeStrengthenedStep(h_k, QPModel, lambda, X_k,
                                                                     activeConstraints, infeasibleConstraints,
                                                                     inner_tol_opt, inner_precision_cst, mu);
            if (inner_tol_opt < inner_precision_cst)
            {
                // We have reached the minimum tolerance and there are still more active constraints
                // than variables: exit
                status = NOMAD::L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS;
                break;
            }
            if (!strengthenedSuccess)
            {
                return L1AugLagSolverStatus::NUM_ERROR;
            }

            // Do a line search along h_k
            const double stepSize = piecewiseLineSearch(X_k, QPModel, h_k, lb, ub, activeConstraints, infeasibleConstraints,
                                                        lambda, mu, 1e-15, 1.5, 1e-7);

            // Update the current iterate
            for (int i = 0; i < n; ++i)
            {
                const double xki = X_k.get(i, 0);
                const double hki = h_k.get(i, 0);
                X_k.set(i, 0, xki + stepSize * hki);
            }
            projectOnBounds(X_k, lb, ub);

            // Recompute parameters for detecting stopping criterion
            QPModelUtils::getModelLagrangianGrad(gradLk, QPModel, X_k, lambda);
            QPModelUtils::getModelCons(cons, QPModel, X_k);
            QPModelUtils::getModelJacobianCons(Jk, QPModel, X_k);
            distXInnerLoop = SGTELIB::Matrix::distNorm2(X_k, X_kp1);
            computeActiveAndInfeasibleSets = true;
            considerMultipliersEstimates = true;

            strenghtenedStepExecuted = true;
            stepSizeLogVal = stepSize;

            continue;
        }

        // Compute a horizontal step
        // The active multipliers must be taken into account
        const bool horizontalSuccess = computeHorizontalStep(h_k, X_k, QPModel, Jk,
                                                             activeConstraints, infeasibleConstraints,
                                                             lambda, lambdaB, mu, true);
        if (!horizontalSuccess)
        {
            return L1AugLagSolverStatus::NUM_ERROR;
        }

        // Set Xcan := P[X_k + h_k]
        for (int i = 0; i < n; ++i)
        {
            const double xi = X_k.get(i, 0);
            const double hi = h_k.get(i, 0);
            Xcan.set(i, 0, xi + hi);
        }
        projectOnBounds(Xcan, lb, ub);

        // Increase feasibility by computing a vertical step. It is computed using active constraint
        // gradients at X_k (and NOT Xcan).
        const bool verticalSuccess = computeVerticalStep(v_k, JkActive, QPModel, Xcan, activeConstraints);
        if (!verticalSuccess)
        {
            return L1AugLagSolverStatus::MATRIX_DIMENSIONS_FAILURE;
        }

        // Set Xcan := P[P[X_k + h_k] + v_k]
        for (int i = 0; i < n; ++i)
        {
            const double xi = Xcan.get(i, 0);
            const double vi = v_k.get(i, 0);
            Xcan.set(i, 0, xi + vi);
        }
        projectOnBounds(Xcan, lb, ub);

        // Compute || ca(x) ||_1, where ca correspond to active constraints
        double nPhi = 0.0;
        for (int j = 0; j < cons.get_nb_rows(); ++j)
        {
            if (!activeConstraints[j])
                continue;

            nPhi += std::abs(cons.get(j, 0));
        }

        // Check if there is a sufficient decrease in the function
        // NB: the sufficient decrease criterion is taken from the original works by
        // T.F. Colemann and A.R. Conn (Fig 1 and Lemma 1, proof).
        const double l1AugLagValX_k = computeL1AugmentedLagrangianVal(QPModel, X_k, lambda, mu);
        const double l1AugLagValXcan = computeL1AugmentedLagrangianVal(QPModel, Xcan, lambda, mu);
        constexpr double sufficient_decrease_tol = 1e-6;
        const bool sufficientDecreaseFound =
                l1AugLagValXcan - l1AugLagValX_k <= -sufficient_decrease_tol * (std::pow(projPGradNorm, 2) + nPhi);
        if (sufficientDecreaseFound)
        {
            // Accept the candidate
            X_k = Xcan;

            // Recompute parameters for detecting stopping criterion
            QPModelUtils::getModelLagrangianGrad(gradLk, QPModel, X_k, lambda);
            QPModelUtils::getModelCons(cons, QPModel, X_k);
            QPModelUtils::getModelJacobianCons(Jk, QPModel, X_k);
            distXInnerLoop = SGTELIB::Matrix::distNorm2(X_k, X_kp1);
            computeActiveAndInfeasibleSets = false;
            considerMultipliersEstimates = false;

            horizontalStepExecuted = true;
            verticalStepExecuted = true;
            stepSizeLogVal = 1.0;

            continue;
        }

        // No sufficient decrease found: compute a strengthened step
        const bool strengthenedSuccess = computeStrengthenedStep(h_k, QPModel, lambda, X_k,
                                                                 activeConstraints, infeasibleConstraints,
                                                                 inner_tol_opt, inner_precision_cst, mu);
        if (inner_precision_cst < min_inner_precision_cst)
        {
            status = NOMAD::L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS;
            break;
        }
        if (!strengthenedSuccess)
        {
            return L1AugLagSolverStatus::NUM_ERROR;
        }

        // Do a line search along h_k
        // Apply a line search along the direction d_k
        const double stepSize = piecewiseLineSearch(X_k, QPModel, h_k, lb, ub, activeConstraints, infeasibleConstraints,
                                                    lambda, mu, 1e-15, 1.5, 1e-7);

        // Update the current iterate
        for (int i = 0; i < n; ++i)
        {
            const double xki = X_k.get(i, 0);
            const double hki = h_k.get(i, 0);
            X_k.set(i, 0, xki + stepSize * hki);
        }
        projectOnBounds(X_k, lb, ub);

        // Recompute parameters for detecting stopping criterion
        QPModelUtils::getModelLagrangianGrad(gradLk, QPModel, X_k, lambda);
        QPModelUtils::getModelCons(cons, QPModel, X_k);
        QPModelUtils::getModelJacobianCons(Jk, QPModel, X_k);
        distXInnerLoop = SGTELIB::Matrix::distNorm2(X_k, X_kp1);
        computeActiveAndInfeasibleSets = true;
        considerMultipliersEstimates = true;

        strenghtenedStepExecuted = true;
        stepSizeLogVal = stepSize;
    }

    if (verbose)
    {
        std::printf("\nStatus: ");
        std::printf("L(x*) = %e\n", QPModelUtils::getModelLagrangian(QPModel, X_k, lambda));
        double normMaxCx0 = 0;
        const int nbCons = cons.get_nb_rows();
        for (int j = 0; j < nbCons; ++j)
        {
            normMaxCx0 = std::max(cons.get(j, 0), normMaxCx0);
        }
        std::printf("|| max(0, c(x*)) ||_inf = %e\n", normMaxCx0);
        if (status == L1AugLagSolverStatus::SOLVED)
        {
            std::printf("Has reached the minimum tolerance:\n");
            std::printf("|| x - P[x - grad Ld(x)] || = %e <= omega = %e or\n",
                        nLdproj, omega);
            std::printf("|| x - P[x - Zk' pseudo grad(x)] || = %e <= tol = %e and\n",
                        projPGradNorm, min_inner_tol_opt);
            std::printf("all constraints below tol = %e and\n", min_inner_precision_cst);
            std::printf("all lambdaB coordinates (|| lambdaB ||_inf = %e) between [-1/mu, 0] = [%e, 0]\n",
                        lambdaB.norm_inf(), -1.0 / mu);
        }
        else if (status == L1AugLagSolverStatus::STAGNATION_ITERATES)
        {
            std::printf("Inner steps have stagnated:\n");
            std::printf("|| x - xp || = %e <= %e\n", distXInnerLoop, tol_dist_successive_x);
            std::printf("step size = %e below tol = 1e-15 \n", stepSizeLogVal);
        }
        else if (status == L1AugLagSolverStatus::MIN_STEPSIZE_REACHED)
        {
            std::printf("Minimum step size has been reached:\n");
            std::printf("step size = %e below tol = 1e-15 \n", stepSizeLogVal);
        }
        else if (status == L1AugLagSolverStatus::MAX_ITER_REACHED)
        {
            std::printf("Max inner iterations reached\n");
        }
        else if (status == L1AugLagSolverStatus::TOO_MANY_ACTIVE_CONSTRAINTS)
        {
            const size_t nbActiveConstraints = std::count(activeConstraints.begin(),
                                                      activeConstraints.end(), true);
            std::printf("At x, %zu constraints are active and ", nbActiveConstraints);
            std::printf("minimum constraint tolerance min tol = %f is reached (tol = %f)\n",
                        min_inner_precision_cst, inner_precision_cst);
        }
        else
        {
            std::printf("Unknown stopping criterion\n");
        }
        std::printf("\n");
    }

    return status;
}


bool NOMAD::L1AugLagSolver::computeHorizontalStep(SGTELIB::Matrix& h_k,
                                                  const SGTELIB::Matrix& X_k,
                                                  const SGTELIB::Matrix& QPModel,
                                                  const SGTELIB::Matrix& Jk,
                                                  const std::vector<bool>& activeConstraints,
                                                  const std::vector<bool>& infeasibleConstraints,
                                                  const SGTELIB::Matrix& lambda,
                                                  const SGTELIB::Matrix& lambdaB,
                                                  const double mu,
                                                  const bool considerActiveMultipliers)
{
    const int n = X_k.get_nb_rows();
    const int nbCons = lambda.get_nb_rows();

    // Compute Z such that JkActive Z = 0; and Z'Z = I
    SGTELIB::Matrix JkActive = extractActiveJacobianCons(Jk, activeConstraints);
    const SGTELIB::Matrix Z = JkActive.null_space();

    // Compute multipliers of the penalized augmented Lagrangian hessian and gradient
    SGTELIB::Matrix multipliers_hessian("multipliers_hess", nbCons, 1);
    SGTELIB::Matrix multipliers_grad("multipliers_grad", nbCons, 1);
    for (int j = 0; j < nbCons; ++j)
    {
        const double lambda_coord = lambda.get(j, 0);
        if (infeasibleConstraints[j])
        {
            multipliers_hessian.set(j, 0, lambda_coord - 1.0 / mu);
            multipliers_grad.set(j, 0, lambda_coord - 1.0 / mu);
        }
        else
        {
            // NB: This point is detailed in Fig. 4.1 of the thesis and its implementation of:
            //
            // Modeles quadratiques et decomposition parallele pour
            // l'optimisation sans derivees
            // by N. Amaioua (2018)
            //
            // In the original paper by T.F. Colemann and A.R. Conn, it is mentioned that when the iterate is
            // close to a first-order solution, one needs to consider the active constraints
            // in the approximation of the Hessian matrix. Here, we directly use the true Hessian of the penalty
            // function.
            if (considerActiveMultipliers && activeConstraints[j])
            {
                multipliers_hessian.set(j, 0, lambda_coord + lambdaB.get(j, 0));
            }
            else
            {
                multipliers_hessian.set(j, 0, lambda_coord);
            }
            multipliers_grad.set(j, 0, lambda_coord);
        }
    }

    // Compute Z'L Z, where L is the Hessian of the penalized augmented Lagrangian.
    SGTELIB::Matrix HLag_k("HLag_k", n, n);
    QPModelUtils::getModelLagrangianHessian(HLag_k, QPModel, X_k, multipliers_hessian);
    const SGTELIB::Matrix ZLZ = SGTELIB::Matrix::product(Z.transpose(), HLag_k, Z);

    // Compute -Z'g, where g is the gradient of the penalized augmented Lagrangian.
    SGTELIB::Matrix GLag_k("GLag_k", n, 1);
    QPModelUtils::getModelLagrangianGrad(GLag_k, QPModel, X_k, multipliers_grad);
    SGTELIB::Matrix ZL = SGTELIB::Matrix::product(Z.transpose(), GLag_k);
    ZL.multiply(-1.0);

    // Solve (Z' L Z) w = - Zt g
    const SGTELIB::Matrix invZLZ = ZLZ.SVD_inverse();
    SGTELIB::Matrix delta_k = SGTELIB::Matrix::product(invZLZ, ZL);

    // h := Z w*, where w is the solution of the linear system above.
    h_k = SGTELIB::Matrix::product(Z, delta_k);

    // if h is not a descent direction, set h:= -Z Z' g, hence h:= -g.
    const double slope = SGTELIB::Matrix::dot(h_k, GLag_k);
    if (slope >= 0)
    {
        h_k = GLag_k;
        h_k.multiply(-1.0);
    }

    return true;
}

bool NOMAD::L1AugLagSolver::computeDropConstraintStep(SGTELIB::Matrix& d,
                                                      const SGTELIB::Matrix& JkActive,
                                                      const SGTELIB::Matrix& lambdaB,
                                                      const SGTELIB::Matrix& pseudoGradient,
                                                      const std::vector<bool>& activeConstraints,
                                                      const double mu)
{
    const int nbCons = lambdaB.get_nb_rows();

    // 1- Check if there exists potential constraints to drop
    int cstToDropId = 0;
    int cstToDropIdLambdaB = -1;
    bool dropConstraintsFound = false;
    for (int i = 0; i < nbCons; ++i)
    {
        if (!activeConstraints[i])
            continue;

        const double lambdaBi = lambdaB.get(i, 0);
        if (lambdaBi > 0 || lambdaBi < (-1.0 / mu))
        {
            cstToDropIdLambdaB = i;
            dropConstraintsFound = true;
            break;
        }
        ++cstToDropId;
    }
    if (!dropConstraintsFound)
        return false;

    // There exists a potential constraint to drop: compute the corresponding new direction.
    // sigma_j0 = -sign(lambdaB[j0]) with j0 = cstToDropId
    const double sigma_j0 = lambdaB[cstToDropIdLambdaB] > 0 ? -1.0 : 1.0;

    SGTELIB::Matrix gradConsToDrop = JkActive.get_row(cstToDropId).transpose();

    // Special case. There is only one active constraint
    // d := sigma_j0 * grad Cjo
    //   := grad Cj0 if lambdaBj0 < - 1/mu
    //      -grad Cj0 if lambdaBj0 > 0
    if (JkActive.get_nb_rows() == 1)
    {
        d = gradConsToDrop;
        d.multiply(sigma_j0);
    }
    else
    {
        // 1- Compute Zkmj: Zkmj satisfies JkActive\j Zkmj = 0 and Zkmj' Zkmj = I, where JkActive\j
        // is the JkActive matrix from which we have removed column j, with j = cstToDropId.
        SGTELIB::Matrix JkActivemj("JkActivemj", JkActive.get_nb_rows() - 1, JkActive.get_nb_cols());
        int k = 0;
        for (int i = 0; i < JkActivemj.get_nb_rows(); ++i)
        {
            if (i != cstToDropId)
            {
                for (int j = 0; j < JkActivemj.get_nb_cols(); ++j)
                {
                    JkActivemj.set(k, j, JkActivemj.get(i, j));
                }
                ++k;
            }
        }
        SGTELIB::Matrix Zkmj = JkActivemj.null_space();

        // 2- Compute new direction d := sigma_j0 Zmj Zmj' JkActive[j]
        //                            := Zmj Zmj' JkActive[j] if lambdaBj < -1/mu
        //                               - Zmj Zmj' JkActive[j] if lambdaBj > 0
        d = SGTELIB::Matrix::product(Zkmj, Zkmj.transpose(), gradConsToDrop);
        d.multiply(sigma_j0);
    }

    // 3- Check if it can be used as a descent direction
    // Compute the dot product of d with grad p + grad Cj0 if lambdaBj0 < -1/mu, grad p if lambdaBj0 > 0.
    if (lambdaB[cstToDropIdLambdaB] > 0)
    {
        gradConsToDrop.fill(0.0);
    }
    gradConsToDrop.add(pseudoGradient);
    const double slope = SGTELIB::Matrix::dot(d, gradConsToDrop);
    constexpr double slope_descent_tol = -1e-6;
    if (slope < slope_descent_tol)
        return true;
    else
        return false;
}

bool NOMAD::L1AugLagSolver::computeStrengthenedStep(SGTELIB::Matrix& h_k,
                                                    const SGTELIB::Matrix& QPModel,
                                                    const SGTELIB::Matrix& lambda,
                                                    const SGTELIB::Matrix& X_k,
                                                    std::vector<bool>& activeConstraints,
                                                    std::vector<bool>& infeasibleConstraints,
                                                    double& inner_tol_opt,
                                                    double& inner_precision_cst,
                                                    const double mu)
{
    constexpr double min_inner_tol_opt = 1e-5;
    constexpr double min_inner_precision_cst = 1e-5;
    const int n = X_k.get_nb_rows();
    const int nbCons = lambda.get_nb_rows();

    // Recompute activeConstraints and infeasibleConstraints
    inner_tol_opt = std::max(inner_tol_opt / 2.0, min_inner_tol_opt);
    inner_precision_cst = std::max(inner_precision_cst / 2.0, min_inner_precision_cst);
    SGTELIB::Matrix cons("cons", nbCons, 1);
    QPModelUtils::getModelCons(cons, QPModel, X_k);
    computeActiveConstraints(activeConstraints, cons, inner_precision_cst);
    computeInfeasibleConstraints(infeasibleConstraints, cons, inner_precision_cst);

    // If the number of active constraints is superior to the number of variables,
    // often the nullspace of the Jacobian of active constraints will have dimension 0.
    // To avoid this case, try to decrease the inner_precision_cst threshold
    int nbActiveConstraints = (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);
    while (nbActiveConstraints >= n)
    {
        inner_precision_cst /= 2.0;
        inner_tol_opt /= 2.0;
        inner_tol_opt = std::max(inner_tol_opt, min_inner_tol_opt);
        if (inner_precision_cst < min_inner_precision_cst)
            break;

        computeActiveConstraints(activeConstraints, cons, inner_precision_cst);
        computeInfeasibleConstraints(infeasibleConstraints, cons, inner_precision_cst);
        nbActiveConstraints = (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);
    }
    if (inner_precision_cst < min_inner_precision_cst)
        return false;

    // Regenerate horizontal step
    SGTELIB::Matrix Jk("Jk", nbCons, n);
    SGTELIB::Matrix lambdaB("lambdaB", nbCons, 1); // It is never used in this case
    QPModelUtils::getModelJacobianCons(Jk, QPModel, X_k);
    const bool horizontalSuccess = computeHorizontalStep(h_k, X_k, QPModel, Jk,
                                                         activeConstraints, infeasibleConstraints,
                                                         lambda, lambdaB, mu, false);

    return horizontalSuccess;
}


bool NOMAD::L1AugLagSolver::computeVerticalStep(SGTELIB::Matrix& v_k,
                                                const SGTELIB::Matrix& JkActive,
                                                const SGTELIB::Matrix& QPModel,
                                                const SGTELIB::Matrix& Xcan,
                                                const std::vector<bool>& activeConstraints)
{
    const int nbCons = (int) activeConstraints.size();
    SGTELIB::Matrix cons("cons", nbCons, 1);
    QPModelUtils::getModelCons(cons, QPModel, Xcan);

    const int nbActive = JkActive.get_nb_rows();
    SGTELIB::Matrix rhs("rhs", nbActive, 1);
    int curInd = 0;
    for (int j = 0; j < nbCons; ++j)
    {
        if (!activeConstraints[j])
            continue;

        rhs.set(curInd, 0, -cons.get(j, 0));
        ++curInd;
    }

    if (curInd != nbActive)
    {
        std::string err = "L1AugLagSolver::solve error: the number of active constraints does ";
        err += "not match the dimensions of the active Jacobian matrix";
        std::printf("%s\n", err.c_str());
        return false;
    }

    // v_k is computed as: v_k := - A_k (A_kt A_k)^-1 activeCons
    // where A_k = JkActive.
    //
    // NB: One could also follow the procedure given in Section 4.1 of
    // "Nonlinear programming via an exact penalty function: Global analysis"
    // by T.F. Coleman and A.R. Conn,  Mathematical Programming 24, 137–161 (1982).
    // https://doi.org/10.1007/BF01585101
    v_k = SGTELIB::Matrix::solve_least_squares_SVD(JkActive, rhs);
    return true;
}

double NOMAD::L1AugLagSolver::piecewiseLineSearch(const SGTELIB::Matrix& X,
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
                                                  const double delta /* = 1E-4 // Pk < (P0 - delta) */)
{
    const int nbCons = lambda.get_nb_rows();
    const int n = X.get_nb_rows();

    // Compute pseudo-gradient
    SGTELIB::Matrix multipliers("multipliers", nbCons, 1);
    for (int j = 0; j < nbCons; ++j)
    {
        const double lambda_coord = lambda.get(j, 0);
        if (infeasibleConstraints[j])
        {
            multipliers.set(j, 0, lambda_coord - 1.0 / mu);
        } else
        {
            multipliers.set(j, 0, lambda_coord);
        }
    }
    SGTELIB::Matrix pseudoGradient("pseudoGradient", n, 1);
    QPModelUtils::getModelLagrangianGrad(pseudoGradient, QPModel, X, multipliers);

    double ak = SGTELIB::Matrix::dot(d, pseudoGradient);
    if (ak >= 0)
    {
        std::printf(
            "L1AugLagSolver::solve warning: in the piecewise line search procedure, the slope should be negative\n");
        return 0.0;
    }

    // Step 1: Compute Ik = { j in {1, ..., nbCons}: |cj(x)| > epsilon and gamma_j = -cj(x) / d' grad cj(x) }
    std::vector<bool> Ik(nbCons);
    std::vector<double> gamma(nbCons, 0.0);

    SGTELIB::Matrix cons("cons", nbCons, 1);
    QPModelUtils::getModelCons(cons, QPModel, X);
    SGTELIB::Matrix Jk("Jk", nbCons, n);
    QPModelUtils::getModelJacobianCons(Jk, QPModel, X);
    SGTELIB::Matrix jprod = SGTELIB::Matrix::product(Jk, d);
    for (int j = 0; j < nbCons; ++j)
    {
        if (!activeConstraints[j])
        {
            if (jprod.get(j, 0) == 0.0)
            {
                // Do not consider the case where gamma = +/-inf
                gamma[j] = 0.0;
                continue;
            }
            gamma[j] = -cons.get(j, 0) / jprod.get(j, 0);
        }
        Ik[j] = (gamma[j] > 0) && (!activeConstraints[j]);
    }

    int nbElementsIk = (int) std::count(Ik.begin(), Ik.end(), true);

    // Step 2 and 3: determine step size gamma_lk
    double gamma_lk = 0;
    while (nbElementsIk != 0 && ak < 0)
    {
        // Find index lk such that lk = argmin_{j in Ik} {gamma: gamma <= gamma_j}
        gamma_lk = NOMAD::INF;
        int lk = -1;
        for (int j = 0; j < nbCons; ++j)
        {
            if (Ik[j] && gamma[j] <= gamma_lk)
            {
                lk = j;
                gamma_lk = gamma[j];
            }
        }

        // Update ak and Ik
        ak += std::abs(jprod[lk]);
        Ik[lk] = false;
        nbElementsIk = (int) std::count(Ik.begin(), Ik.end(), true);
    }

    // Step 5: check the new iterate satisfies a sufficient decrease, otherwise perform a line search
    // to find a better point
    SGTELIB::Matrix X_k("X_k", n, 1);
    for (int i = 0; i < n; ++i)
    {
        X_k.set(i, 0, X.get(i, 0) + gamma_lk * d.get(i, 0));
    }
    projectOnBounds(X_k, lb, ub);
    const double P0 = computeL1AugmentedLagrangianVal(QPModel, X, lambda, mu);
    double Pk = computeL1AugmentedLagrangianVal(QPModel, X_k, lambda, mu);

    bool obtainSufficientDecrease = Pk < (P0 - delta);
    if (obtainSufficientDecrease)
    {
        return gamma_lk;
    }

    // Apply a quadratic interpolation (and not a cubic interpolation as it is described in
    // the original algorithm)
    const double quad_search_step_size = std::min(1.0,
                                                  quadraticInterpolationSearch(X, QPModel, d, lb, ub, lambda , mu));
    for (int i = 0; i < n; ++i)
    {
        X_k.set(i, 0, X.get(i, 0) + quad_search_step_size * d.get(i, 0));
    }
    projectOnBounds(X_k, lb, ub);
    Pk = computeL1AugmentedLagrangianVal(QPModel, X_k, lambda, mu);

    obtainSufficientDecrease = Pk < (P0 - delta);
    if (obtainSufficientDecrease)
    {
        return quad_search_step_size;
    }

    // Last tentative if it does not work: apply an Armijo linesearch
    while (!obtainSufficientDecrease)
    {
        gamma_lk /= gamma_update;
        for (int i = 0; i < n; i++)
        {
            X_k.set(i, 0, X.get(i, 0) + gamma_lk * d.get(i, 0));
        }
        Pk = computeL1AugmentedLagrangianVal(QPModel, X_k, lambda, mu);
        obtainSufficientDecrease = (Pk < (P0 - delta)) || (gamma_lk <= small_gamma);
    }

    return std::max(gamma_lk, small_gamma);
}

double NOMAD::L1AugLagSolver::quadraticInterpolationSearch(const SGTELIB::Matrix& X,
                                                           const SGTELIB::Matrix& QPModel,
                                                           const SGTELIB::Matrix& d,
                                                           const SGTELIB::Matrix& lb,
                                                           const SGTELIB::Matrix& ub,
                                                           const SGTELIB::Matrix& lambda,
                                                           const double mu)
{
    constexpr double min_threshold = 1e-12;

    // Determine maximum step size allowed
    double c = [](const SGTELIB::Matrix& X,
                  const SGTELIB::Matrix& d,
                  const SGTELIB::Matrix& lb,
                  const SGTELIB::Matrix& ub)
    {
        const int n = d.get_nb_rows();

        double max_step_size = NOMAD::INF;
        for (int i = 0; i < n; ++i)
        {
            const double li = lb.get(i, 0);
            const double ui = ub.get(i, 0);
            max_step_size = std::min(ui - li, max_step_size);
        }

        for (int i = 0; i < n; ++i)
        {
            const double di = d.get(i, 0);
            if (di == 0.0)
                continue;

            const double li = lb.get(i, 0);
            const double ui = ub.get(i, 0);
            const double xi = X.get(i, 0);
            if (li != -NOMAD::INF)
            {
                if (di < 0.0)
                {
                    const double tmp_step_size = (xi - li) / std::abs(di);
                    max_step_size = std::min(max_step_size, tmp_step_size);
                    continue;
                }
            }
            if (ui != NOMAD::INF)
            {
                if (di > 0.0)
                {
                    const double tmp_step_size = (ui - xi) / std::abs(di);
                    max_step_size = std::min(max_step_size, tmp_step_size);
                }
            }
        }
        return max_step_size;
    }(X, d, lb, ub);
    double b = c / 2.0; // Middle bound
    double a = 0.0; // Lower bound of the interval for the determination of the step size
    double alpha = (b + c) / 2.0;

    // Initialize points and values in interval
    const int n = X.get_nb_rows();
    SGTELIB::Matrix Xa("Xa", n, 1);
    Xa = d; Xa.multiply(a); Xa.add(X);
    double fxa = computeL1AugmentedLagrangianVal(QPModel, Xa, lambda, mu);
    SGTELIB::Matrix Xb("Xb", n, 1);
    Xb = d; Xb.multiply(b); Xb.add(X);
    double fxb = computeL1AugmentedLagrangianVal(QPModel, Xb, lambda, mu);
    SGTELIB::Matrix Xc("Xc", n, 1);
    Xc = d; Xc.multiply(c); Xc.add(X);
    double fxc = computeL1AugmentedLagrangianVal(QPModel, Xc, lambda, mu);
    SGTELIB::Matrix Xalpha("Xalpha", n, 1);
    Xalpha = d; Xalpha.multiply(alpha); Xalpha.add(X);
    double fxalpha = computeL1AugmentedLagrangianVal(QPModel, Xalpha, lambda, mu);

    while (abs(alpha - b) > min_threshold)
    {
        // Update interval bounds
        if ((alpha - b) * (alpha - c) < 0)
        {
            if (fxalpha < fxb)
            {
                a = b;
                b = alpha;
                fxa = fxb;
                fxb = fxalpha;
            }
            else
            {
                c = alpha;
                fxc = fxalpha;
            }
        }
        else
        {
            if (fxalpha < fxb)
            {
                c = b;
                b = alpha;
                fxc = fxb;
                fxb = fxalpha;
            }
            else
            {
                a = alpha;
                fxa = fxalpha;
            }
        }
        const double A = 2.0 * (fxa * (b - c) + fxb * (c - a) + fxc * (a - b));
        if (std::abs(A) <= min_threshold)
        {
            alpha = b;
            fxalpha = fxb;
        }
        else
        {
            // Interpolation
            alpha = (fxa * (b * b - c * c) + fxb * (c * c - a * a) + fxc * (a * a - b * b)) / A;
            if ((alpha - a) * (alpha - c) < 0)
            {
                Xalpha = d; Xalpha.multiply(alpha); Xalpha.add(X);
                fxalpha = computeL1AugmentedLagrangianVal(QPModel, Xalpha, lambda, mu);
            }
            else
            {
                alpha = b;
                fxalpha = fxb;
            }
        }
    }
    return alpha;
}

bool NOMAD::L1AugLagSolver::checkDimensions(const SGTELIB::Matrix& x,
                                            const SGTELIB::Matrix& QPModel,
                                            const SGTELIB::Matrix& lb,
                                            const SGTELIB::Matrix& ub)
{
    const int n = x.get_nb_rows();
    if (n != std::max(x.get_nb_rows(), x.get_nb_cols()) && (x.get_nb_cols() != 1))
    {
        std::string err = "L1AugLagSolver::solve error: x must be a column vector";
        std::printf("%s\n", err.c_str());
        return false;
    }

    if (n != lb.get_nb_rows() || n != ub.get_nb_rows())
    {
        std::string err = "L1AugLagSolver::solve error: bound constraints dimensions ";
        err += "nlb = " + std::to_string(lb.get_nb_cols()) + " nub = " + std::to_string(ub.get_nb_cols());
        err += " are not compatible with dimension of x (n = " + std::to_string(n) + ")";
        std::printf("%s\n", err.c_str());
        return false;
    }

    const int nbParams = QPModel.get_nb_cols();
    if (nbParams != (n + 1) + n * (n + 1) / 2)
    {
        std::string err = "L1AugLagSolver::solve error: ";
        err += "the number of params of the model nbParams = (n+1) * (n+2) / 2 = " + std::to_string(nbParams);
        err += " is not compatible with the dimension of the solution n = " + std::to_string(n);
        std::printf("%s\n", err.c_str());
        return false;
    }

    const int nbCons = QPModel.get_nb_rows() - 1;
    if (nbCons < 1)
    {
        std::string err = "L1AugLagSolver::solve error: ";
        err += "the model has no constraints";
        std::printf("%s\n", err.c_str());
        return false;
    }

    return true;
}

bool NOMAD::L1AugLagSolver::checkBoundsCompatibilities(const SGTELIB::Matrix& lb,
                                                       const SGTELIB::Matrix& ub)
{
    const int n = lb.get_nb_cols();
    for (int i = 0; i < n; ++i)
    {
        const bool areBoundsCompatible = lb.get(i, 0) <= ub.get(i, 0);
        if (!areBoundsCompatible)
        {
            std::string err = "L1AugLagSolver::solve error: ";
            err += "no compatibility between lower bound and upper bound for index " + std::to_string(i);
            std::printf("%s\n", err.c_str());
            return false;
        }
    }
    return true;
}

void NOMAD::L1AugLagSolver::projectOnBounds(SGTELIB::Matrix& x,
                                            const SGTELIB::Matrix& lb,
                                            const SGTELIB::Matrix& ub)
{
    const int n = x.get_nb_rows();
    for (int i = 0; i < n; ++i)
    {
        const double xi = std::clamp(x.get(i, 0), lb.get(i, 0), ub.get(i, 0));
        x.set(i, 0, xi);
    }
}

double NOMAD::L1AugLagSolver::computeFirstOrderError(const SGTELIB::Matrix& x,
                                                     const SGTELIB::Matrix& gradLk,
                                                     const SGTELIB::Matrix& lb,
                                                     const SGTELIB::Matrix& ub)
{
    const int n = x.get_nb_rows();
    SGTELIB::Matrix dualFeas("dualFeas", n, 1);
    for (int i = 0; i < n; ++i)
    {
        dualFeas.set(i, 0, x.get(i, 0) - gradLk.get(i, 0));
    }
    projectOnBounds(dualFeas, lb, ub);
    dualFeas.sub(x);

    return dualFeas.norm_inf();
}

void NOMAD::L1AugLagSolver::computeActiveConstraints(std::vector<bool>& activeConstraints,
                                                     const SGTELIB::Matrix& cons,
                                                     const double inner_tol)
{
    const int nbCons = cons.get_nb_rows();
    for (int j = 0; j < nbCons; ++j)
    {
        const double ci = cons.get(j, 0);
        activeConstraints[j] = std::abs(ci) <= inner_tol;
    }
}

void NOMAD::L1AugLagSolver::computeInfeasibleConstraints(std::vector<bool>& infeasibleConstraints,
                                                         const SGTELIB::Matrix& cons,
                                                         const double inner_tol)
{
    const int nbCons = cons.get_nb_rows();
    for (int j = 0; j < nbCons; ++j)
    {
        const double ci = cons.get(j, 0);
        infeasibleConstraints[j] = ci > inner_tol;
    }
}

void NOMAD::L1AugLagSolver::computeMultipliersInfeasibleConstraints(SGTELIB::Matrix& lambda,
                                                                    const SGTELIB::Matrix& gradL,
                                                                    const SGTELIB::Matrix& J,
                                                                    const std::vector<bool>& activeConstraints,
                                                                    const std::vector<bool>& infeasibleConstraints,
                                                                    const double mu)
{
    lambda.fill(0.0);
    const int nbActiveCons =  (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);

    // In this case, we assume the multipliers are all equal to zero
    if (nbActiveCons == 0)
        return;

    // Compute grad D(x, mu) = grad L(x) + (1 / mu) sum_V cj(x)
    // where V is the set of infeasible constraints indexes
    const int n = gradL.get_nb_rows();
    SGTELIB::Matrix gradD("gradD", n, 1);
    computePseudoGradient(gradD, gradL, J, infeasibleConstraints, mu);

    // Compute lambda solution of min || J_A(x)' lambda - grad D(x, mu) ||
    // where A is the set of active constraints
    SGTELIB::Matrix JActive = extractActiveJacobianCons(J, activeConstraints);
    auto JActiveT = JActive.transpose();

    // 1- Initialize matrices for SVD decomposition
    const int nbCons = JActiveT.get_nb_cols();
    auto U = new double *[n];
    auto W = new double[nbCons];
    auto V = new double *[nbCons];
    for (int i = 0; i < n; ++i)
    {
        U[i] = new double[nbCons];
    }
    for (int i = 0; i < nbCons; ++i)
    {
        V[i] = new double[nbCons];
    }

    // 2- Compute the SVD of the transposed Jacobian matrix
    std::string error_msg;
    JActiveT.SVD_decomposition(error_msg, U, W, V, 1000000000);

    int rank = 0;
    constexpr double rank_tol = 1e-15;
    for (int i = 0; i < nbCons; i++)
    {
        if (fabs(W[i]) > rank_tol)
        {
            rank++;
        } else
        {
            W[i] = 0;
        }
    }

    // 3- Solve the least-square subproblem
    SGTELIB::Matrix Wm = SGTELIB::Matrix("Wm", nbCons, nbCons);
    for (int i = 0; i < nbCons; i++)
    {
        for (int j = 0; j < nbCons; j++)
        {
            const double Wmi = (i == j) && W[i] != 0 ? 1.0 / (W[i] * W[i]) : 0;
            Wm.set(i, j, Wmi);
        }
    }
    SGTELIB::Matrix Vm("Vm", nbCons, nbCons, V);
    SGTELIB::Matrix sol = SGTELIB::Matrix::product(Wm, Vm.transpose(), JActive, gradD);
    sol = SGTELIB::Matrix::product(Vm, sol);

    // For each constraint that is active, set lambda[j] := sol[j]
    int curInd = 0;
    for (int j = 0; j < lambda.get_nb_rows(); ++j)
    {
        if (!activeConstraints[j])
            continue;

        lambda.set(j, 0, sol.get(curInd, 0));
        curInd++;
    }

    for (int i = 0; i < n; i++)
    {
        delete [] U[i];
    }
    delete [] U;
    for (int j = 0; j < nbCons; j++)
    {
        delete [] V[j];
    }
    delete [] V;
    delete [] W;
}


void NOMAD::L1AugLagSolver::computePseudoGradient(SGTELIB::Matrix& gradD,
                                                  const SGTELIB::Matrix& gradL,
                                                  const SGTELIB::Matrix& J,
                                                  const std::vector<bool>& infeasibleConstraints,
                                                  const double mu)
{
    const int n = gradL.get_nb_rows();
    const int nbCons = (int) infeasibleConstraints.size();

    gradD = gradL;
    SGTELIB::Matrix Jcomponent("Jcomponent", n, 1);
    for (int j = 0; j < nbCons; ++j)
    {
        if (!infeasibleConstraints[j])
            continue;

        for (int i = 0; i < n; ++i)
        {
            Jcomponent.set(i, 0, J.get(j, i));
        }
        Jcomponent.multiply(1.0 / mu);
        gradD.add(Jcomponent);
    }
}

void NOMAD::L1AugLagSolver::computeGradLd(SGTELIB::Matrix& gradLd,
                                          const SGTELIB::Matrix& gradL,
                                          const SGTELIB::Matrix& J,
                                          const SGTELIB::Matrix& lambdaB,
                                          const std::vector<bool>& activeConstraints,
                                          const std::vector<bool>& infeasibleConstraints,
                                          const double mu)
{
    computePseudoGradient(gradLd, gradL, J, infeasibleConstraints, mu);

    const int n = J.get_nb_cols();
    const int nbCons = J.get_nb_rows();
    SGTELIB::Matrix Jcomponent("Jcomponent", n, 1);
    for (int j = 0; j < nbCons; ++j) {
        if (!activeConstraints[j])
            continue;

        for (int i = 0; i < n; ++i) {
            Jcomponent.set(i, 0, J.get(j, i));
        }
        Jcomponent.multiply(-lambdaB.get(j, 0));
        gradLd.add(Jcomponent);
    }
}

SGTELIB::Matrix NOMAD::L1AugLagSolver::extractActiveJacobianCons(const SGTELIB::Matrix& J,
                                                                 const std::vector<bool>& activeConstraints)
{
    const int n = J.get_nb_cols();
    const int nbCons = J.get_nb_rows();
    const int nbActiveCons = (int) std::count(activeConstraints.begin(), activeConstraints.end(), true);

    SGTELIB::Matrix JActive("JActive", nbActiveCons, n);
    int curInd = 0;
    for (int i = 0; i < nbCons; ++i)
    {
        if (!activeConstraints[i])
            continue;

        for (int j = 0; j < n; ++j)
        {
            JActive.set(curInd, j, J.get(i, j));
        }
        curInd++;
    }
    return JActive;
}

double NOMAD::L1AugLagSolver::computeL1AugmentedLagrangianVal(const SGTELIB::Matrix& QPModel,
                                                              const SGTELIB::Matrix& x,
                                                              const SGTELIB::Matrix& lambda,
                                                              const double mu)
{
    double l1AugLagVal = QPModelUtils::getModelLagrangian(QPModel, x, lambda);

    const int nbCons = lambda.get_nb_rows();
    for (int j = 0; j < nbCons; ++j)
    {
        const double cj = QPModelUtils::getModelCons(QPModel, j, x);
        l1AugLagVal += (1.0 / mu) * std::max(0.0, cj);
    }
    return l1AugLagVal;
}
