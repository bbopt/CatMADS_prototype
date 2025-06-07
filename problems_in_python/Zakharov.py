import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["linear", "floor", "sign"],
        ["linear", "floor", "sign"]
    ],
    "x_int": [
        (-3, 3),
        (-3, 3)
    ],
    "x_con": [
        (-5.0, 5.0)
    ] * 4
}

def zakharov(X):
    """
    Zakharov-like problem with conditional perturbations on both linear and nonlinear terms.

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,) — zero for unconstrained problems
        g : list of empty lists — one per sample
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    i1 = X_int[:, 0][:, None]
    i2 = X_int[:, 1][:, None]
    X_con = X_con.astype(float)
    n = X_con.shape[1]

    # --- Compute p(x) and s(x) vectors ---
    p_mod = np.zeros_like(X_con)
    s_mod = np.zeros_like(X_con)

    for j in range(n):
        x = X_con[:, j]

        # Compute p(x)
        p_mod[:, j] = np.select(
            [cat1 == "linear", cat1 == "floor", cat1 == "sign"],
            [
                0.1 * (i1[:, 0] - i2[:, 0] + x),
                0.05 * np.floor(i1[:, 0] + i2[:, 0] + x),
                -0.1 * np.sqrt(x + 5) * (i1[:, 0] + i2[:, 0])
            ],
            default=0.0
        )

        # Compute s(x)
        s_mod[:, j] = np.select(
            [cat2 == "linear", cat2 == "floor", cat2 == "sign"],
            [
                0.1 * (-i1[:, 0] + i2[:, 0] - x),
                0.05 * np.floor(-i1[:, 0] - i2[:, 0] - x),
                0.1 * np.sqrt(x + 5) * (i1[:, 0] + i2[:, 0])
            ],
            default=0.0
        )

    # --- Compute Zakharov components ---
    weights = np.arange(1, n + 1) / 2.0  # shape (4,)
    term1 = np.sum(X_con**2, axis=1)
    term2 = np.sum(weights * (X_con + p_mod), axis=1) ** 2
    term3 = np.sum(weights * (X_con + s_mod), axis=1) ** 4

    f = term1 + term2 + term3 + 1
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]
    return f, h, g
