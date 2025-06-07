import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [["absolute", "quad", "logsum", "hyperbol", "invgauss"]],
    "x_int": [(-5, 10), (-5, 10)],
    "x_con": [(-5.0, 10.0), (-5.0, 10.0)]
}

def styblinskitang_constrained(X):
    """
    Constrained Styblinski–Tang problem with mixed variables.

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
    int1 = X_int[:, 0]
    int2 = X_int[:, 1]
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]

    def compute_p(cat, x):
        return np.select(
            [cat == "absolute", cat == "quad", cat == "logsum", cat == "hyperbol", cat == "invgauss"],
            [
                np.abs(x - 1),
                (x**2 + 1) / 2.0,
                np.exp(np.abs(x + 1)) - 1,
                x**2 / (1 + np.abs(x)),
                1 - np.exp(-x**2)
            ],
            default=np.nan
        )

    p1 = compute_p(cat, x1)
    p2 = compute_p(cat, x2)

    f = 0.5 * ((x1**4 - 16 * x1**2 + 5 * int1 + 8 * p1) +
               (x2**4 - 16 * x2**2 + 5 * int2 + 8 * p2))

    g1 = np.full_like(x1, np.nan)
    g2 = np.full_like(x1, np.nan)

    # Constraints by category
    for label in np.unique(cat):
        idx = cat == label
        if label == "absolute":
            g1[idx] = np.exp(x1[idx] + x2[idx]) - 10
            g2[idx] = (x1[idx] + 2)**3 + (x2[idx] - 1)**2 + 0.1 * (int1[idx] - 1)**2 - 50
        elif label == "quad":
            g1[idx] = np.exp(x1[idx] + 2 * x2[idx]) - 18
            g2[idx] = (x1[idx] - 1)**3 + (x2[idx] + 2)**2 + 0.1 * (int1[idx] + 2)**2 - 40
        elif label == "logsum":
            g1[idx] = np.exp(2 * x1[idx] + x2[idx]) - 18
            g2[idx] = x1[idx]**3 + (x2[idx] - 2)**2 + 0.1 * (int1[idx] - 3)**2 - 45
        elif label == "hyperbol":
            g1[idx] = np.exp(x1[idx] - x2[idx]) + np.log(1 + np.abs(x2[idx])) - 12
            g2[idx] = np.sin((x1[idx] + x2[idx]) / 10) + x1[idx]**2 + 0.2 * np.abs(int1[idx] - 1) - 3
        elif label == "invgauss":
            g1[idx] = np.exp(x1[idx] - 0.5 * x2[idx]) + np.log(1 + np.abs(x2[idx])) - 11
            g2[idx] = np.sin((x1[idx] + x2[idx]) / 12.5) + x1[idx]**2 + 0.2 * np.abs(int1[idx] - 2) - 4

    g = [[g1[i], g2[i]] for i in range(X.shape[0])]
    h = np.array([sum(max(0, gj)**2 for gj in gk) for gk in g])

    return f, h, g
