import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [["A", "B", "C", "D", "E", "F"]],
    "x_int": [(-25, 25)],
    "x_con": [(-25.0, 25.0)] * 3
}


def cat_cstrs_5(X):
    """
    Cat-cstrs-5 based on the EVD-52 problem.

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,)
        g : list of lists — each containing one constraint value per sample
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    cat = X_cat[:, 0]
    i = X_int[:, 0]
    x1, x2, x3 = X_con.T

    # Objective (negated)
    f = np.select(
        [
            cat == "A",
            cat == "B",
            cat == "C",
            cat == "D",
            cat == "E",
            cat == "F"
        ],
        [
            -1 * (x1**2 + x2**2 + x3**2 - 1 - i / 50),
            -1 * (x1**2 + x2**2 + (x3 - 2)**2 - i / 50),
            -1 * (x1 + x2 + x3 - 1 - i / 50),
            -1 * (x1 + x2 - x3 + 1 + i / 50),
            -1 * (2 * x1**3 + 6 * x2**2 + 2 * (5 * x3 - x1 + 1)**2 + i / 50),
            -1 * (x1**2 - 9 * x3 + i / 50)
        ],
        default=np.nan
    )

    # Constraint g(x) ≤ 0
    g0 = np.select(
        [
            cat == "A",
            cat == "B",
            cat == "C",
            cat == "D",
            cat == "E",
            cat == "F"
        ],
        [
            np.abs(x1 + x2 - 5)**1.5 + (x3 + 1)**2 + np.abs(i) / 10 - 30,
            (x1 * x2)**2 + np.abs(x3 - 1) + np.abs(i - 5) / 10 - 30,
            np.abs(x1 - x2)**3 + x3**2 + np.abs(i + 5) / 10 - 30,
            (x1 + x2)**2 + np.abs(x3 - 0.5)**1.5 + np.abs(i - 10) / 10 - 30,
            (x1 - 2 * x2)**2 + (x3 - 1.5) + np.abs(i + 10) / 10 - 30,
            (x1 * x3)**2 + np.abs(x2 + 1.5) + np.abs(i - 15) / 10 - 30
        ],
        default=np.nan
    )

    g = [[g0[k]] for k in range(X.shape[0])]
    h = np.array([max(0, gj[0])**2 for gj in g])

    return f, h, g
