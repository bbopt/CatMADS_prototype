import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B"],
        ["A", "B"]
    ],
    "x_int": [
        (83, 93),
        (90, 95),
        (3, 12)
    ],
    "x_con": [
        (1e-5, 2000),
        (1e-5, 16000),
        (1e-5, 120),
        (1e-5, 5000),
        (1e-5, 2000)
    ]
}


def cat_cstrs_9(X):
    """
    Cat-cstrs-9 based on the HS-144 problem.

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
    i1, i2, i3 = X_int.T
    x1, x2, x3, x4, x5 = X_con.T

    # Compute p_val
    term_p = 1.12 * x1 + 0.13167 * i3 * x1 - 0.00667 * i3**2 * x1 - (1 / 0.99) * x4
    p_val = np.where(cat1 == "A", -500 * term_p, 500 * term_p)

    # Compute s_val
    term_s = 1.098 * i3 - 0.038 * i3**2 + 0.325 * i1 - (1 / 0.99) * i2 + 57.425
    s_val = np.where(cat2 == "A", -500 * term_s, 500 * term_s)

    # Objective
    f = (
        -5.04 * x1
        - 0.035 * x2
        - 10 * x3
        - 3.36 * x5
        + 0.063 * i2 * x4
        + p_val
        + s_val
    )

    # Constraints
    g1 = 0.02 * x2 + 0.1 * x5 - 100
    g2 = -0.1 * x2 - 0.5 * x5 + 100
    g3 = -0.5 * x4 + 0.1 * x1 + 500
    g4 = 0.2 * x4 - 0.2 * x1 - 500

    g = [[g1[k], g2[k], g3[k], g4[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
