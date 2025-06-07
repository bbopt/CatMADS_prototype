import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [["A", "B", "C"]],
    "x_int": [(-3, 3), (-3, 3)],
    "x_con": [(-np.pi, np.pi)] * 4
}

def pentagon_constrained(X):
    """
    Pentagon constrained problem involving Euclidean distances and angle-based inequality constraints.

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

    cat = X_cat[:, 0]
    i1, i2 = X_int.T
    x1, x2, x3, x4 = X_con.T

    # Objective based on category
    f = np.empty(X.shape[0])
    mask_A = cat == "A"
    mask_B = cat == "B"
    mask_C = cat == "C"

    if np.any(mask_A):
        f[mask_A] = np.sqrt((x1[mask_A] - i1[mask_A])**2 + (x2[mask_A] - i2[mask_A])**2)
    if np.any(mask_B):
        f[mask_B] = np.sqrt((i1[mask_B] - x3[mask_B])**2 + (i2[mask_B] - x4[mask_B])**2)
    if np.any(mask_C):
        f[mask_C] = np.sqrt((x3[mask_C] - x1[mask_C])**2 + (x4[mask_C] - x2[mask_C])**2)

    # Constraint angles
    angle1 = 2 * np.pi / 5
    angle2 = 8 * np.pi / 5

    # Constraints (g1–g6 ≤ 0)
    g1 = i1 * np.cos(angle1) + i2 * np.sin(angle1) - 1
    g2 = i1 * np.cos(angle2) + i2 * np.sin(angle2) - 1
    g3 = x1 * np.cos(angle1) + x2 * np.sin(angle1) - 1
    g4 = x1 * np.cos(angle2) + x2 * np.sin(angle2) - 1
    g5 = x3 * np.cos(angle1) + x4 * np.sin(angle1) - 1
    g6 = x3 * np.cos(angle2) + x4 * np.sin(angle2) - 1

    g = [[g1[k], g2[k], g3[k], g4[k], g5[k], g6[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
