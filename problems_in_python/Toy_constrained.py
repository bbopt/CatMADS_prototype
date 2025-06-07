import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [["A", "B", "C", "D", "E", "F", "G", "H", "I", "J"]],
    "x_int": [],
    "x_con": [(0.0, 1.0)] * 4
}

def toy_constrained(X):
    """
    Constrained Toy problem.

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
    x1, x2, x3, x4 = X_con[:, 0], X_con[:, 1], X_con[:, 2], X_con[:, 3]
    f = np.full(X.shape[0], 5.0)

    for label in np.unique(cat):
        idx = cat == label
        if label == "A":
            f[idx] += np.cos(3.6 * np.pi * (x1[idx] - 2) + x2[idx]) + x3[idx] - 1 + x4[idx]**2
        elif label == "B":
            f[idx] += 2 * np.cos(1.1 * np.pi * np.exp(x1[idx])) - x2[idx]/2 + x3[idx]**2 + 2 * np.log(1 + x4[idx]**2)
        elif label == "C":
            f[idx] += np.cos(2 * np.pi * x1[idx]) + x2[idx]/2 + x3[idx] * x4[idx]
        elif label == "D":
            f[idx] += x1[idx] * np.cos(3.4 * np.pi * (x1[idx] - 1)) - x2[idx] - 1 + x3[idx] + x4[idx]**3
        elif label == "E":
            f[idx] += -(x1[idx]**2)/2 + np.log(1 + x2[idx]**2) + x3[idx]**2 + x4[idx]
        elif label == "F":
            f[idx] += 2 * np.cos((np.pi / 4) * np.exp(-(x1[idx]**4)))**2 - x2[idx]/2 + x3[idx] * x4[idx] + 1
        elif label == "G":
            f[idx] += x1[idx] * np.cos(3.4 * x1[idx]) - x2[idx]/2 + x3[idx] + x4[idx]**3 + 1
        elif label == "H":
            f[idx] += x1[idx] * (-np.cos(7 / (2 * np.pi)) * x2[idx] / 2) + x3[idx] + x4[idx] + 2
        elif label == "I":
            f[idx] += -(x1[idx]**3)/2 + x2[idx]**2 + x3[idx] * x4[idx] + 1
        elif label == "J":
            f[idx] += -(np.cos(5 * np.pi * x1[idx])**2) * np.sqrt(x1[idx]) - (-np.log(x2[idx] + x3[idx] + 0.5)) / 2 + x4[idx]**3 - 1.3

    norm = np.sqrt(x1**2 + x2**2 + x3**2 + x4**2)
    g1 = norm - 0.25**2
    g2 = -norm + 0.1**2
    g = [[g1[i], g2[i]] for i in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
