import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [["A", "B", "C", "D", "E", "F"]],
    "x_int": [(0, 10)] * 4,
    "x_con": [(0.0, 10.0)] * 6
}


def cat_cstrs_16(X):
    """
    Cat-cstrs-16 based on the Wong-2 problem.

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

    f = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]
    cat = X_cat[:, 0]
    i1, i2, i3, i4 = X_int[:, 0], X_int[:, 1], X_int[:, 2], X_int[:, 3]
    x1, x2, x3, x4, x5, x6 = [X_con[:, i] for i in range(6)]

    # Penalty
    p_val = (
        x1**2 + i1**2 + x1 * i1 - 14 * x1 - 16 * i1
        + (x2 - 10)**2 + 4 * (i2 - 5)**2 + (x3 - 3)**2
        + 2 * (i3 - 1)**2 + 5 * x4 + 7 * (i4 - 11)**2
        + 2 * (x5 - 10)**2 + (x6 - 7)**2 + 45
    )
    f = -p_val

    # Apply category-based terms
    for label in np.unique(cat):
        idx = cat == label
        if label == "B":
            f[idx] += 10 * (3 * (x1[idx] - 2)**2 + 4 * (i1[idx] - 3)**2 + 2 * x2[idx]**2 - 7 * i2[idx]**2 - 120)
        elif label == "C":
            f[idx] += 10 * (5 * x1[idx]**2 + 8 * i1[idx] + 6 * (x2[idx] - 6)**2 - 2 * i2[idx] - 40)
        elif label == "D":
            f[idx] += 10 * (0.5 * (x1[idx] - 8)**2 + 2 * (i1[idx] - 4)**2 + 3 * x3[idx]**2 - i3[idx] - 30)
        elif label == "E":
            f[idx] += 10 * (x1[idx]**2 + 2 * (i1[idx] - 2)**2 - 2 * x1[idx] * i1[idx] + 14 * x3[idx] - 6 * i3[idx])
        elif label == "F":
            f[idx] += 10 * (-3 * x1[idx] + 6 * i1[idx] + 12 * (x5[idx] - 8)**2 - 7 * x6[idx])

    # Constraint definitions
    for j in range(X.shape[0]):
        c = cat[j]
        if c == "A":
            g[j] = [
                4 * x1[j] + 5 * i1[j] - 3 * x4[j] + 9 * i4[j] - 105,
                10 * x1[j] - 8 * i1[j] - 17 * x4[j] + 2 * i4[j],
                -8 * x1[j] + 2 * i1[j] + 5 * x5[j] - 2 * x6[j]
            ]
        elif c == "B":
            g[j] = [
                3 * x1[j] + 6 * i1[j] - 3 * x4[j] + 9 * i4[j] - 105,
                8 * x1[j] - 6 * i1[j] - 17 * x4[j] + 2 * i4[j],
                -4 * x1[j] + 4 * i1[j] + 5 * x5[j] - 2 * x6[j]
            ]
        elif c == "C":
            g[j] = [
                4 * x1[j] + 4 * i1[j] - 2 * x4[j] + 9 * i4[j] - 105,
                10 * x1[j] - 10 * i1[j] - 15 * x4[j] + 2 * i4[j],
                -8 * x1[j] + i1[j] + 10 * x5[j] - 2 * x6[j]
            ]
        elif c == "D":
            g[j] = [
                4 * x1[j] + 5 * i1[j] - 4 * x4[j] + 10 * i4[j] - 105,
                10 * x1[j] - 8 * i1[j] - 19 * x4[j] + 4 * i4[j],
                -8 * x1[j] + 2 * i1[j] + 2.5 * x5[j] - 4 * x6[j]
            ]
        elif c == "E":
            g[j] = [
                5 * x1[j] + 5 * i1[j] - 3 * x4[j] + 8 * i4[j] - 105,
                12 * x1[j] - 8 * i1[j] - 19 * x4[j] + i4[j],
                -16 * x1[j] + 2 * i1[j] + 5 * x5[j] - x6[j]
            ]
        elif c == "F":
            g[j] = [
                3 * x1[j] + 4 * i1[j] - x4[j] + 8 * i4[j] - 95,
                9 * x1[j] - 9 * i1[j] - 18 * x4[j] + i4[j] + 10,
                -4 * x1[j] + i1[j] + 10 * x5[j] - 4 * x6[j] + 10
            ]

    h = np.array([sum(max(0, gk)**2 for gk in g[j]) for j in range(X.shape[0])])
    return f, h, g
