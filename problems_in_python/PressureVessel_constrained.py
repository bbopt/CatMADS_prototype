import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [["A", "B", "C", "D", "E", "F", "G", "H"]],
    "x_int": [(1, 99), (1, 99)],
    "x_con": [(10.0, 200.0), (10.0, 200.0)]
}

def pressurevessel_constrained(X):
    """
    Pressure vessel design problem (constrained), using categorical nonlinear shape mappings.

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
    int1 = 0.0625 * X_int[:, 0]
    int2 = 0.0625 * X_int[:, 1]
    x1, x2 = X_con.T  # x_con1, x_con2

    # Compute p(cat, x2) vectorized
    p_val = np.zeros(X.shape[0])
    for i, cat in enumerate(cat1):
        x = x2[i]
        if cat == "A":
            p_val[i] = 20 + 0.8 * x + 15 * np.sin(0.02 * x)
        elif cat == "B":
            p_val[i] = 50 + 0.85 * x + 18 * np.sin(0.025 * x + 0.3)
        elif cat == "C":
            p_val[i] = 10 + 0.75 * x + 12 * np.sin(0.018 * x - 0.4)
        elif cat == "D":
            p_val[i] = 40 + 0.82 * x + 20 * np.sin(0.022 * x + 0.5)
        elif cat == "E":
            p_val[i] = 200 - 0.8 * x - 15 * np.sin(0.02 * x)
        elif cat == "F":
            p_val[i] = 160 - 0.85 * x - 18 * np.sin(0.025 * x + 0.3)
        elif cat == "G":
            p_val[i] = 220 - 0.75 * x - 12 * np.sin(0.018 * x - 0.4)
        elif cat == "H":
            p_val[i] = 180 - 0.82 * x - 20 * np.sin(0.022 * x + 0.5)
        else:
            raise ValueError("Invalid x_cat1")

    # Objective function
    f = (
        0.6224 * int1 * x1 * p_val
        + 1.7781 * int2 * p_val**2
        + 3.1661 * int1**2 * x1
        + 19.84 * int1**2 * p_val
    )

    # Constraints
    g1 = -int1 + 0.0193 * p_val
    g2 = -int2 + 0.00954 * p_val
    g3 = -np.pi * x1 * p_val**2 - (4 * np.pi / 3) * p_val**3 + 1296000

    g = [[g1[k], g2[k], g3[k]] for k in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
