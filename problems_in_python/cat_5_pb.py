import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["0", "1", "2", "3", "4", "5"]
    ],
    "x_int": [
        (-25, 25)
    ],
    "x_con": [
        (-25.0, 25.0),
        (-25.0, 25.0),
        (-25.0, 25.0)
    ]
}


def cat_5(X):
    """
    Cat-5 based on the EVD-52 problem.

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

    # Define each branch of the piecewise function
    f0 = x1**2 + x2**2 + x3**2 - 1 - x_int / 50
    f1 = x1**2 + x2**2 + (x3 - 2)**2 - x_int / 50
    f2 = x1 + x2 + x3 - 1 - x_int / 50
    f3 = x1 + x2 - x3 + 1 + x_int / 50
    f4 = 2 * x1**3 + 6 * x2**2 + 2 * (5 * x3 - x1 + 1)**2 + x_int / 50
    f5 = x1**2 - 9 * x3 + x_int / 50

    f = np.select(
        [
            cat == "0",
            cat == "1",
            cat == "2",
            cat == "3",
            cat == "4",
            cat == "5"
        ],
        [f0, f1, f2, f3, f4, f5]
    )

    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return f, h, g
