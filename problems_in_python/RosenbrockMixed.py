import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["smooth", "nonsmooth"],
        ["A", "B", "C"]
    ],
    "x_int": [
        (-2, 2),
        (-5, 5)
    ],
    "x_con": [
        (-10.0, 10.0)
    ] * 4
}

def rosenbrockmixed(X):
    """
    RosenbrockMixed problem with smooth/nonsmooth variants and category-based penalty.

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

    cat1 = X_cat[:, 0]           # smooth vs nonsmooth
    cat2 = X_cat[:, 1]           # A, B, C
    int1 = X_int[:, 0]           # for |x_int1|
    sum_con = np.sum(X_con, axis=1)
    f = np.abs(int1).astype(float)  # base term f = |x_int1|

    # Penalty term
    penalty = np.select(
        [cat2 == "A", cat2 == "B", cat2 == "C"],
        [
            (1.1 / 4.0) * np.maximum(0.0, sum_con),
            (-0.9 / 4.0) * np.minimum(0.0, sum_con),
            np.mean(np.abs(X_con), axis=1)
        ]
    )
    f += penalty

    # Rosenbrock component
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]
    x3 = X_con[:, 2]
    x4 = X_con[:, 3]

    if np.any(cat1 == "smooth"):
        mask = cat1 == "smooth"
        f[mask] += (
            100 * (x2[mask] - x1[mask]**2)**2 + (x1[mask] - 1)**2 +
            100 * (x3[mask] - x2[mask]**2)**2 + (x2[mask] - 1)**2 +
            100 * (x4[mask] - x3[mask]**2)**2 + (x3[mask] - 1)**2
        )

    if np.any(cat1 == "nonsmooth"):
        mask = cat1 == "nonsmooth"
        f[mask] += (
            100 * np.abs(x2[mask] - x1[mask]**2) + 5 * np.abs(x1[mask] - 1) +
            100 * np.abs(x3[mask] - x2[mask]**2) + 5 * np.abs(x2[mask] - 1) +
            100 * np.abs(x4[mask] - x3[mask]**2) + 5 * np.abs(x3[mask] - 1)
        )

    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]
    return f, h, g
