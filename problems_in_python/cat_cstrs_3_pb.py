import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C"],
        ["A", "B", "C"]
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


def cat_cstrs_3(X):
    """
    Cat-cstrs-3 based on the Bukin-6 problem.

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

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    i1 = X_int[:, 0]
    i2 = X_int[:, 1]
    x1, x2, x3, x4 = X_con.T

    # Compute p
    p_val = np.select(
        [cat1 == "A", cat1 == "B", cat1 == "C"],
        [
            np.sqrt(np.abs(x2 + x3) + 2.0),
            np.abs(x2 + x3),
            (np.power(x2 + x3, 2) / 1.25) + 1.0
        ]
    )

    # Compute h
    h_val = np.select(
        [cat2 == "A", cat2 == "B", cat2 == "C"],
        [
            np.sqrt(np.abs(i1 + x1 + x4) + 1.5),
            np.abs(i1 + x1 + x4),
            ((i1 + np.power(x1 + x4, 2)) / 1.25) + 1.0
        ]
    )

    # Objective
    f = 100.0 * np.sqrt(np.abs(p_val - 0.01 * h_val)) + 0.01 * np.abs(h_val + i2)

    # Constraints
    g1 = np.select(
        [cat1 == "A", cat1 == "B", cat1 == "C"],
        [
            1.5 * np.sin((x1 + x4) / 5) + 0.5 * i1 - 0.1 * x1**2 - 2.0,
            1.5 * np.sin((x1 - x4) / 5) + 0.3 * i1 - 0.1 * x4**2 - 2.5,
            np.exp(0.2 * x1 + 0.1 * x4) + 0.1 * i1 - 4.0
        ]
    )

    g2 = np.select(
        [cat2 == "A", cat2 == "B", cat2 == "C"],
        [
            1.5 * np.cos((x2 + x3) / 5) + 0.4 * i2 - 0.2 * x2**2 - 1.5,
            1.5 * np.cos((x2 - x3) / 5) + 0.6 * i2 - 0.2 * x3**2 - 2.0,
            np.log(1 + (x2 + x3)**2) + 0.2 * i2 - 3.0
        ]
    )

    # Combine constraints
    g = [[g1[k], g2[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
