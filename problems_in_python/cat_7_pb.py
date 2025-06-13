import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["quad", "abs"],
        ["quad", "abs"],
        ["A", "B", "C", "D"]
    ],
    "x_int": [
        (-2, 2),
        (-2, 2),
        (-2, 2)
    ],
    "x_con": [
        (-2.0, 2.0),
        (-2.0, 2.0)
    ]
}


def cat_7(X):
    """
    Cat-7 based on the Goldstein-Price problem.

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
    cat3 = X_cat[:, 2]
    int1 = X_int[:, 0]
    int2 = X_int[:, 1]
    int3 = X_int[:, 2]
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]

    # --- s_val (4-way case split on cat1 × cat2) ---
    conds_s = [
        (cat1 == "quad") & (cat2 == "quad"),
        (cat1 == "quad") & (cat2 == "abs"),
        (cat1 == "abs") & (cat2 == "quad"),
        (cat1 == "abs") & (cat2 == "abs")
    ]
    s_vals = [
        2 + ((x1 + int1) ** 2 + (x2 + int2) ** 2) / 2.0,
        1.5 + ((x1 + int1) ** 2 + np.abs(x2 + int2)) / 4.0,
        1.5 + (np.abs(x1 + int1) + (x2 + int2) ** 2) / 4.0,
        1 + (np.abs(x1 + int1) + np.abs(x2 + int2))
    ]
    s_val = np.select(conds_s, s_vals, default=1.0)

    # --- p_val (based on cat3) ---
    conds_p = [
        cat3 == "A",
        cat3 == "B",
        cat3 == "C",
        cat3 == "D"
    ]
    p_vals = [
        (np.abs(int3 + x2) + 2) / 2.0,
        (np.abs(int3 - x2) + 2) / 2.0,
        (np.abs(-int3 + x2) + 2) / 2.0,
        (np.abs(-int3 - x2) + 2) / 2.0
    ]
    p_val = np.select(conds_p, p_vals, default=1.0)

    # --- Goldstein-Price function ---
    term1 = (
        1 + (x1 + x2 + 1)**2 *
        (19 - 14 * x1 + 3 * x1**2 - 14 * x2 + 6 * x1 * x2 + 3 * x2**2)
    )
    term2 = (
        30 + (2 * x1 - 3 * x2)**2 *
        (18 - 32 * x1 + 12 * x1**2 + 48 * x2 - 36 * x1 * x2 + 27 * x2**2)
    )
    gold = term1 * term2

    f = gold + s_val + p_val
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return f, h, g
