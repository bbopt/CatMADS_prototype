/**
 \file   QPModelUtils.hpp
 \brief  Utilities functions for Quadratic Model
 \author Ludovic Salomon
 \see    QPModelUtils.cpp
 */
#ifndef __NOMAD_4_5_QPMODEL_UTILS__
#define __NOMAD_4_5_QPMODEL_UTILS__

#include "../../../ext/sgtelib/src/Matrix.hpp"

#include "../../nomad_nsbegin.hpp"

/// Contains utilities to deal with the QP model matrix, which MUST HAVE the following form:
///     [ m11 m12 ... m1q ]
/// M = [ m21 m22 ... m2q ]
///     [       :         ]
///     [ mp1 mp2 ... mpq ]
/// where q = n + 1 + n (n + 1) / 2 the number of variables of the problem
/// and p is the number of outputs of the blackbox.
/// - The first line MUST contain the parameters of the quadratic model of the objective function.
/// - The other lines contain the parameters of the quadratic models of the constraint functions, if they exist.
///
/// For example, coefficient of the first line of the QP model matrix correspond to:
/// Qf(x) = alpha0 + alphaL' x + (1/2) x' Hq x
/// with alpha0 = m11, alphaL = [m12 ... m1(n+1)]' and
///      [ m1(n+2)     x       x     x     ]
/// Hq = [ m1(2n+2) m1(n+3)    x     x     ]
///      [ m1(2n+3) m1(2n+4)   .     x     ]
///      [  ...      ...      m1q m1(2n+1) ]
class QPModelUtils
{
public:
    static double getModelObj(const SGTELIB::Matrix& QPModel,
                              const SGTELIB::Matrix& x);

    static void getModelObjGrad(SGTELIB::Matrix& g,
                                const SGTELIB::Matrix& QPModel,
                                const SGTELIB::Matrix& x);

    static void getModelObjHessian(SGTELIB::Matrix& H,
                                   const SGTELIB::Matrix& QPModel,
                                   const SGTELIB::Matrix& x);

    static double getModelCons(const SGTELIB::Matrix& QPModel,
                               const int ind,
                               const SGTELIB::Matrix& x);

    static void getModelCons(SGTELIB::Matrix& cons,
                             const SGTELIB::Matrix& QPModel,
                             const SGTELIB::Matrix& x);

    static void getModelJacobianCons(SGTELIB::Matrix& jacobian,
                                     const SGTELIB::Matrix& QPModel,
                                     const SGTELIB::Matrix& x);

    static void getModelConsHessian(SGTELIB::Matrix& H,
                                    const SGTELIB::Matrix& QPModel,
                                    const int ind,
                                    const SGTELIB::Matrix& x);

    // Functions for the computation of the Lagrangian function:
    // L(x, lambda) = sigma f(x) - lambda' c(x)
    static double getModelLagrangian(const SGTELIB::Matrix& QPModel,
                                     const SGTELIB::Matrix& x,
                                     const SGTELIB::Matrix& lambda,
                                     const double sigma = 1.0);

    static void getModelLagrangianGrad(SGTELIB::Matrix& gradL,
                                       const SGTELIB::Matrix& QPModel,
                                       const SGTELIB::Matrix& x,
                                       const SGTELIB::Matrix& lambda,
                                       const double sigma = 1.0);

    static void getModelLagrangianHessian(SGTELIB::Matrix& H,
                                          const SGTELIB::Matrix& QPModel,
                                          const SGTELIB::Matrix& x,
                                          const SGTELIB::Matrix& lambda,
                                          const double sigma = 1.0);

    static SGTELIB::Matrix getReducedQPModel(const SGTELIB::Matrix& QPModel,
                                             const SGTELIB::Matrix& x,
                                             const std::vector<bool>& fixedVars);

private:
    static double getModelValue(const SGTELIB::Matrix& QPModel,
                                const int ind,
                                const SGTELIB::Matrix& x);

    static void getModelGrad(SGTELIB::Matrix& g,
                             const SGTELIB::Matrix& QPModel,
                             const int ind,
                             const SGTELIB::Matrix& x);

    static void getModelHessian(SGTELIB::Matrix& H,
                                const SGTELIB::Matrix& QPModel,
                                const int ind,
                                const SGTELIB::Matrix& x);
};

#include "../../nomad_nsend.hpp"

#endif //__NOMAD_4_5_QPMODEL_UTILS__
