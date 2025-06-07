import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C", "D", "E"],
        ["A", "B", "C", "D", "E"]
    ],
    "x_int": [
        (0, 5),
        (0, 5)
    ],
    "x_con": [
        (-5.0, 5.0),
        (-5.0, 5.0)
    ]
}

def himmelblau_constrained(X):
    """
    Constrained Himmelblau problem with category-mapped weights and two inequality constraints.

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
    x1, x2 = X_con.T

    # Lookup values for p and s
    p_map = {"A": 0.75, "B": 1.5, "C": 2.25, "D": 3.0, "E": 3.75}
    s_map = {"A": -1.25, "B": -0.5, "C": 0.25, "D": 1.0, "E": 1.75}
    p = np.array([p_map[c] for c in cat1])
    s = np.array([s_map[c] for c in cat2])

    # Objective (translated Himmelblau)
    f = (x1**2 + x2 - 6 - i1)**2 + (x1 + x2**2 - 2 - i2)**2 + 10

    # Constraints
    g1 = -p * x1 - x2 + 0.5
    g2 = 3 * x1 - s * x2 + 1

    g = [[g1[k], g2[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
