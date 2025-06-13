import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["root", "absolute", "quadratic"],
        ["linear", "absolute", "quadratic"]
    ],
    "x_int": [
        (-5, 5),
        (-5, 5)
    ],
    "x_con": [
        (-15.0, 5.0),
        (-3.0, 3.0),
        (-15.0, 5.0),
        (-3.0, 3.0)
    ]
}


def cat_4(X):
    """
    Cat-4 based on the Bukin-6 problem.

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
    int1 = X_int[:, 0]
    int2 = X_int[:, 1]
    con1 = X_con[:, 0]
    con2 = X_con[:, 1]
    con3 = X_con[:, 2]
    con4 = X_con[:, 3]

    # p(cat1, con2, con3)
    sum23 = con2 + con3
    p_val = np.select(
        [cat1 == "root", cat1 == "absolute", cat1 == "quadratic"],
        [np.sqrt(np.abs(sum23) + 2.0),
         np.abs(sum23),
         (sum23 ** 2) / 1.25 + 1.0]
    )

    # h(cat2, int1, con1, con4)
    h_input = int1 + con1 + con4
    h_val = np.select(
        [cat2 == "linear", cat2 == "absolute", cat2 == "quadratic"],
        [np.sqrt(np.abs(h_input) + 1.5),
         np.abs(h_input),
         (int1 + (con1 + con4) ** 2) / 1.25 + 1.0]
    )

    # Objective function
    f = 100.0 * np.sqrt(np.abs(p_val - 0.01 * h_val)) + 0.01 * np.abs(h_val + int2)
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return f, h, g
