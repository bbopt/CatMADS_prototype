import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C", "D"]
    ],
    "x_int": [
        (0, 5)
    ],
    "x_con": [
        (-2.0, 2.0),
        (-2.0, 2.0),
        (-2.0, 2.0),
        (-2.0, 2.0),
        (-2.0, 2.0)
    ]
}

def hs78(X):
    """
    HS78 problem (product + category-based function over continuous variables).

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
    x4 = X_con[:, 3]
    x5 = X_con[:, 4]

    # Compute product across each row
    product = np.prod(X_con, axis=1)

    # Compute p_val using categorical rule
    conds = [
        cat == "A",
        cat == "B",
        cat == "C",
        cat == "D"
    ]

    p_vals = [
        x1**2 + x2**2 + x3**2 + x4**2 + x5**2 - 10.0,
        x2 * x3 - 5.0 * x4 * x5,
        x1**3 + x2**3 + 1.0,
        0.5 * (x2 * x3 - 5.0 * x4 * x5 + x1**3 + x2**3 + 1.0)
    ]

    p_val = np.select(conds, p_vals, default=0.0)

    f = product + x_int * p_val
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return f, h, g
