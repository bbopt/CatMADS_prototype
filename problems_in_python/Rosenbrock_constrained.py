import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [["smooth", "nonsmooth"], ["A", "B", "C"]],
    "x_int": [(-2, 2), (-5, 5)],
    "x_con": [(-10.0, 10.0)] * 4
}

def rosenbrock_constrained(X):
    """
    Constrained variant of the Rosenbrock function with mixed-variable inputs.

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
    int1 = X_int[:, 0]
    int2 = X_int[:, 1]

    x1, x2, x3, x4 = X_con.T

    # Penalty based on x_cat2
    p_val = np.zeros(X.shape[0])
    for i in range(X.shape[0]):
        if cat2[i] == "A":
            p_val[i] = np.mean([1.1 * max(0, x) for x in X_con[i]])
        elif cat2[i] == "B":
            p_val[i] = np.mean([-0.9 * min(0, x) for x in X_con[i]])
        else:  # "C"
            p_val[i] = np.mean([abs(x) for x in X_con[i]])

    # Objective
    f = np.abs(int1) + p_val
    for i in range(X.shape[0]):
        con = X_con[i]
        if cat1[i] == "smooth":
            f[i] += sum(
                100 * (con[j + 1] - con[j] ** 2) ** 2 + int2[i] * (con[j] - 1) ** 2
                for j in range(3)
            )
        else:  # nonsmooth
            f[i] += sum(
                100 * abs(con[j + 1] - con[j] ** 2) + 5 * int2[i] * abs(con[j] - 1)
                for j in range(3)
            )

    # Constraint g1
    g1 = -np.linalg.norm(X_con, axis=1) + (int1 / 2) ** 2
    g1 += np.select(
        [cat2 == "A", cat2 == "B", cat2 == "C"],
        [4.25**2, 5.5**2, 8**2]
    )

    g = [[g1[i]] for i in range(X.shape[0])]
    h = np.array([(max(0, g1i))**2 for g1i in g1])

    return f, h, g
