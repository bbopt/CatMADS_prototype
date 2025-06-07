import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["max", "absolute", "indicator"],
        ["linear", "divide", "quadratic"]
    ],
    "x_int": [
        (-5, 5),
        (-2, 2)
    ],
    "x_con": [
        (-5.12, 5.12)
    ] * 8
}

def rastragin(X):
    """
    Rastrigin problem (modified) with categorical logic for scaling and masking.

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

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    int1 = X_int[:, 0][:, None]  # (n_samples, 1)
    int2 = X_int[:, 1]

    X_con = X_con.astype(float)  # (n_samples, 8)
    x_shifted = int1 + X_con     # (n_samples, 8)

    # --- Compute p_val ---
    p_val = np.zeros(X.shape[0])

    mask_max = cat1 == "max"
    mask_abs = cat1 == "absolute"
    mask_ind = cat1 == "indicator"

    if np.any(mask_max):
        p_val[mask_max] = np.mean(np.maximum(0.0, x_shifted[mask_max]) ** 2, axis=1)

    if np.any(mask_abs):
        p_val[mask_abs] = np.mean(np.abs(x_shifted[mask_abs]) / 2.0, axis=1)

    if np.any(mask_ind):
        xcon_m = X_con[mask_ind]
        xsh_m = x_shifted[mask_ind]
        p_tmp = []
        for xi, xci in zip(xsh_m, xcon_m):
            p_tmp.append(
                np.sum([
                    max(v, 0.0) * abs(c)
                    for v, c in zip(xi, xci) if v > 0
                ])
            )
        p_val[mask_ind] = np.array(p_tmp) / 8.0

    # --- Compute s_val ---
    s_val = np.select(
        [cat2 == "linear", cat2 == "divide", cat2 == "quadratic"],
        [
            1.0 + 0.1 * int2,
            1.0 / (1.0 + 0.1 * int2),
            1.0 + 0.05 * (int2 ** 2)
        ],
        default=1.0
    )

    # --- Modified Rastrigin function ---
    f = 10.0 * p_val + np.sum(X_con**2 - s_val[:, None] + np.cos(X_con), axis=1)

    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return f, h, g
