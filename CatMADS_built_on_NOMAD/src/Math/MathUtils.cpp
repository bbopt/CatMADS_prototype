#include "../Math/MathUtils.hpp"

#include <cmath>

bool NOMAD::roots_quadratic(const double q2, const double q1, const double q0,
                            double& r1, double& r2)
{
    if (q2 == 0)
    {
        if (q1 == 0)
        {
            r1 = r2 = 0;
            if (q0 != 0)
            {
                return false;
            }
        }
        else
        {
            r1 = r2 = -q0 / q1;
        }
        return true;
    }

    // q is quadratic
    const double tol = 1e-8; // =~ sqrt(eps(double))
    const double rhs = tol * q1 * q1;
    if (std::abs(q0 * q2) > rhs)
    {
        const double rho = q1 * q1 - 4 * q2 * q0;
        if (rho < 0)
        {
            return false;
        }
        const double numd2 = -(q1 + std::copysign(sqrt(rho), q1)) / 2.0;
        r1 = numd2 / q2;
        r2 = q0 / numd2;
    }
    else
    {
        // Ill-conditioned quadratic
        r1 = -q1 / q2;
        r2 = 0;
    }

    // Improve accuracy with one newton iteration
    const size_t niter = 1;
    for (size_t i = 0; i < niter; ++i)
    {
        const double q = (q2 * r1 + q1) * r1 + q0;
        const double dq = 2 * q2 * r1 + q1;
        if (dq == 0)
            continue;

        r1 -= q / dq;
    }

    for (size_t i = 0; i < niter; ++i)
    {
        const double q = (q2 * r2 + q1) * r2 + q0;
        const double dq = 2 * q2 * r2 + q1;
        if (dq == 0)
            continue;

        r2 -= q / dq;
    }

    return true;
}
