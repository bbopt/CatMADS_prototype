import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C"],
        ["A", "B", "C"]
    ],
    "x_int": [
        (-10, 10),
        (-10, 10)
    ],
    "x_con": [
        (-10.0, 10.0),
        (-10.0, 10.0),
        (-10.0, 10.0)
    ]
}


def g09_constrained(X):
    """
    G09 constrained problem with lookup-table-based coefficients and 4 nonlinear constraints.

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,)
        g : list of lists — constraint values per sample
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    i1 = X_int[:, 0]
    i2 = X_int[:, 1]
    x1, x2, x3 = X_con.T

    # Lookup dictionaries for p and s
    table_p = {
        ("A", "A"): -5, ("A", "B"): 2.5, ("A", "C"): 7.5,
        ("B", "A"): -5, ("B", "B"): 3.5, ("B", "C"): 8.5,
        ("C", "A"): -4, ("C", "B"): 4.5, ("C", "C"): 10
    }

    table_s = {
        ("A", "A"): -5, ("A", "B"): -5, ("A", "C"): -4,
        ("B", "A"): 2.5, ("B", "B"): 4.5, ("B", "C"): 3,
        ("C", "A"): 1, ("C", "B"): 6.5, ("C", "C"): 9
    }

    # Vectorized table lookup
    keys = list(zip(cat1, cat2))
    p = np.array([table_p[k] for k in keys])
    s = np.array([table_s[k] for k in keys])

    # Objective function
    f = (
        (x1 - 10) ** 2 +
        5 * (x2 - 12) ** 2 +
        x3 ** 4 +
        3 * (i1 - 11) ** 2 +
        10 * i2 ** 6 +
        7 * p ** 2 +
        s ** 2 -
        4 * p * s -
        10 * p -
        8 * s
    )

    # Constraints
    g1 = (2 * x1**2 + 3 * x2**4 + x3 + 4 * i1**2 + 5 * i2) / 2 - 127
    g2 = (7 * x1 + 3 * x2 + 10 * x3**2 + i1 - i2) / 2 - 282
    g3 = 23 * x1 + x2**2 + p - 8 * s - 196
    g4 = 4 * x1**2 + x2**2 - 3 * x1 * x2 + 2 * x3**2 + 5 * p - 11 * s

    g = [[g1[k], g2[k], g3[k], g4[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
