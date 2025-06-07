import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C", "D"]
    ],
    "x_int": [
        (0, 5)
    ],
    "x_con": [
        (-5.0, 5.0)
    ] * 4
}

def rosensuzuki(X):
    """
    Rosen-Suzuki problem with categorical-dependent penalty terms.

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,) — zero for unconstrained problems
        g : list of empty lists — one per sample, for uniformity with constrained problems
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    cat = X_cat[:, 0]
    x_int = X_int[:, 0]
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]
    x3 = X_con[:, 2]
    x4 = X_con[:, 3]

    # Base p(x_int, x_con)
    p_val = (
        x1**2 + x2**2 + 2 * x3**2 + x4**2
        - 5 * x1 - 5 * x2 - 21 * x3 + 7 * x4
        + x_int * x3
    )

    f = np.copy(p_val)

    # Case-by-case penalty additions
    mask_B = cat == "B"
    mask_C = cat == "C"
    mask_D = cat == "D"

    if np.any(mask_B):
        f[mask_B] += 10 * (
            x1[mask_B]**2 + x2[mask_B]**2 + x3[mask_B]**2 + x4[mask_B]**2 +
            x1[mask_B] - x2[mask_B] + x3[mask_B] - x4[mask_B] - x_int[mask_B] - 8
        )

    if np.any(mask_C):
        f[mask_C] += 10 * (
            x1[mask_C]**2 + 2 * x2[mask_C]**2 + x3[mask_C]**2 + 2 * x4[mask_C]**2 -
            x1[mask_C] - x4[mask_C] - 2 * x_int[mask_C] - 10
        )

    if np.any(mask_D):
        f[mask_D] += 10 * (
            2 * x1[mask_D]**2 + x2[mask_D]**2 + x3[mask_D]**2 + 2 * x4[mask_D]**2 -
            x1[mask_D] - x2[mask_D] - 3 * x_int[mask_D] - 5
        )

    f += 100
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]
    return f, h, g
