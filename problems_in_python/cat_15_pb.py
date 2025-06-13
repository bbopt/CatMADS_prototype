import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C", "D", "E"]
    ],
    "x_int": [
        (-1, 1),
        (-1, 1),
        (-1, 1)
    ],
    "x_con": [
        (-1.0, 1.0),
        (-1.0, 1.0),
        (-1.0, 1.0),
        (-1.0, 1.0)
    ]
}


def cat_15(X):
    """
    Cat-15 based on the Wong-1 problem.

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
    i1 = X_int[:, 0]
    i2 = X_int[:, 1]
    i3 = X_int[:, 2]
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]
    x3 = X_con[:, 2]
    x4 = X_con[:, 3]

    # Base function p(...)
    f = (
        (x1 - 10)**2 +
        5 * (i1 - 12)**2 +
        x2**4 +
        3 * (i2 - 11)**2 +
        10 * x3**6 +
        7 * i3**2 +
        x4**4 -
        4 * i3 * x4 -
        10 * i3 -
        8 * x4
    )

    # Category-based penalty terms
    f += np.select(
        [
            cat == "B",
            cat == "C",
            cat == "D",
            cat == "E"
        ],
        [
            10 * (2 * x1**2 + 3 * i1**4 + x2 + i2**2 + 5 * x3 - 127),
            10 * (7 * x1 + 3 * i1 + 10 * x2**3 + i2 - x3 - 282),
            10 * (23 * x1 + i1**2 + 6 * i3 - 8 * x4 - 196),
            10 * (4 * x1**2 + i1 - 3 * x1 * i1 + 2 * x2**2 + 5 * i3 - 11 * x4)
        ],
        default=0.0
    )

    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]
    return f, h, g
