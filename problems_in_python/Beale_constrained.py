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

def beale_constrained(X):
    """
    Constrained Beale-like problem.

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,) — constraint aggregation
        g : list of lists — individual constraints per sample
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    i1 = X_int[:, 0]
    i2 = X_int[:, 1]
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]
    x3 = X_con[:, 2]

    # g(cat1, x1)
    g_val = np.select(
        [cat1 == "A", cat1 == "B", cat1 == "C"],
        [np.floor(x1), x1, np.exp(x1 / 2)],
        default=0.0
    )

    # h(cat2, x2)
    h_val = np.select(
        [cat2 == "A", cat2 == "B", cat2 == "C"],
        [np.sqrt(np.abs(x2) + 1), np.abs(x2), x2**2 - 2],
        default=0.0
    )

    term1 = (1.5 - g_val + i1 * (1 - h_val))**2
    term2 = (2.25 - g_val + i2 * (1 - h_val)**2)**2
    term3 = (2.625 - g_val + x3 * (1 - h_val)**3)**2
    f = term1 + term2 + term3

    # Constraints g_i(x) ≤ 0
    g1 = 3 * (x1 - 2)**2 + 4 * (x2 - 3)**2 + 2 * x3 - 100
    g2 = 5 * x1**2 + 8 * x2 + (x3 - 6)**2 - 30
    g3 = x1**2 + 2 * (x2 - 2)**2 - 2 * x1 * x2 - 6 * x3

    # Stack constraints per sample
    g = [[g1[k], g2[k], g3[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
