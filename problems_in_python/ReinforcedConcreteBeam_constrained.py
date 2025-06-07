import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [["A", "B", "C", "D", "E"], ["A", "B", "C", "D", "E"]],
    "x_int": [(28, 40)],
    "x_con": [(5.0, 10.0), (5.0, 10.0)]
}

def reinforcedconcretebeam_constrained(X):
    """
    Reinforced Concrete Beam design problem (constrained).

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,)
        g : list of lists — individual constraint values per sample
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    i1 = X_int[:, 0]
    x1, x2 = X_con.T

    # Category → index
    cat_map = {"A": 0, "B": 1, "C": 2, "D": 3, "E": 4}
    idx1 = np.vectorize(cat_map.get)(cat1)
    idx2 = np.vectorize(cat_map.get)(cat2)

    # Lookup table for p(cat1, cat2)
    p_table = np.array([
        [0.2,  0.52, 0.83, 1.13, 1.45],
        [0.27, 0.58, 0.87, 1.19, 1.49],
        [0.33, 0.63, 0.91, 1.24, 1.54],
        [0.38, 0.68, 0.96, 1.30, 1.62],
        [0.42, 0.73, 1.01, 1.35, 1.66]
    ])
    p_val = p_table[idx2, idx1]

    # Objective
    f = 29.4 * p_val + 0.6 * i1 * (x1 + x2)

    # Constraints
    g1 = 5 * (x1 + x2) - 4 * i1 + 2 * p_val - 2.5
    g2 = 2.5 * p_val**2 + 25 * i1 - p_val * i1 * (x1 + x2) - 2.5

    g = [[g1[k], g2[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
