/**
 \file   DoglegTRSolver.hpp
 \brief  The Dogleg Trust-region method
 \author Ludovic Salomon
 \see    DoglegTRSolver.cpp
 */
#ifndef NOMAD_DOGLEGTRSOLVER_H
#define NOMAD_DOGLEGTRSOLVER_H

#include "../../../ext/sgtelib/src/Matrix.hpp"

#include "../../nomad_nsbegin.hpp"

enum class DoglegTRSolverStatus
{
    MATRIX_DIMENSIONS_FAILURE, ///< Problem with matrix dimensions
    QR_FACTORIZATION_FAILURE, ///< QR factorization has failed
    TR_NUM_ERROR, ///< Trust-region numerical error
    TR_PARAM_ERROR, ///< Trust-region parameter error
    SOLVED ///< Problem solved
};

/// Dogleg Trust-Region solver
/// Solve:
/// min || A x + b ||^2
/// s.t  |x| <= delta
class DoglegTRSolver
{
public:
    static DoglegTRSolverStatus solve(SGTELIB::Matrix& x,
                                      const SGTELIB::Matrix& A,
                                      const SGTELIB::Matrix& b,
                                      const double delta);
};

#include "../../nomad_nsend.hpp"

#endif //NOMAD_DOGLEGTRSOLVER_H
