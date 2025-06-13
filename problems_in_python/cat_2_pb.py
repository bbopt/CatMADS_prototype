import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C"],
        ["A", "B", "C"]
    ],
    "x_int": [
        (-2, 2),
        (-2, 2)
    ],
    "x_con": [
        (-4.5, 4.5),
        (-4.5, 4.5),
        (-4.5, 4.5)
    ]
}


def cat_2(X):
    """
    Cat-2 based on the Beale problem.

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

    # Unpack variables
    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    int1 = X_int[:, 0]
    int2 = X_int[:, 1]
    con1 = X_con[:, 0]
    con2 = X_con[:, 1]
    con3 = X_con[:, 2]

    # Compute g(cat1, con1)
    g_val = np.select(
        [cat1 == "A", cat1 == "B", cat1 == "C"],
        [np.floor(con1), con1, np.exp(con1 / 2.0)]
    )

    # Compute h(cat2, con2)
    h_val = np.select(
        [cat2 == "A", cat2 == "B", cat2 == "C"],
        [np.sqrt(np.abs(con2) + 1), np.abs(con2), con2 ** 2 - 2]
    )

    term1 = (1.5   - g_val + int1 * (1 - h_val))       ** 2
    term2 = (2.25  - g_val + int2 * (1 - h_val**2))    ** 2
    term3 = (2.625 - g_val + con3 * (1 - h_val**3))    ** 2

    f = term1 + term2 + term3
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return f, h, g
