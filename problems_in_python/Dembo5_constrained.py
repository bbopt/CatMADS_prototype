import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C", "D"]
    ],
    "x_int": [
        (10, 1000),
        (10, 1000),
        (10, 1000),
        (10, 1000)
    ],
    "x_con": [
        (100.0, 10000.0),
        (1000.0, 10000.0),
        (1000.0, 10000.0),
        (10.0, 1000.0)
    ]
}

def dembo5_constrained(X):
    """
    Dembo5 constrained problem with large-scale expressions and 3 constraints.

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,) — aggregated constraint violation
        g : list of lists — individual constraint values per sample
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    cat = X_cat[:, 0]
    i1, i2, i3, i4 = X_int.T
    x1, x2, x3, x4 = X_con.T

    # Base objective
    f = -(x1 + x2 + x3)

    # Category-specific penalty
    mask_B = cat == "B"
    mask_C = cat == "C"
    mask_D = cat == "D"

    if np.any(mask_B):
        f[mask_B] -= 1e5 * (
            833.33252 * x4[mask_B] / (x1[mask_B] * i2[mask_B]) +
            100 / i2[mask_B] -
            83333.333 / (x1[mask_B] * i2[mask_B]) -
            1
        )

    if np.any(mask_C):
        f[mask_C] -= 1e5 * (
            x4[mask_C] / i3[mask_C] +
            1250 * (i1[mask_C] - x4[mask_C]) / (x2[mask_C] * i3[mask_C]) -
            1
        )

    if np.any(mask_D):
        f[mask_D] -= 1e5 * (
            1250000 / (x3[mask_D] * i4[mask_D]) +
            i1[mask_D] / i4[mask_D] -
            2500 * i1[mask_D] / (x3[mask_D] * i4[mask_D]) -
            1
        )

    # Constraints
    g1 = 0.0025 * (i2 + x4) - 1
    g2 = 0.0025 * (i1 + i3 + x4) - 1
    g3 = 0.01 * (i4 - i1) - 1

    g = [[g1[k], g2[k], g3[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
