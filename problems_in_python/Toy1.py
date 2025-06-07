import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J"]
    ],
    "x_int": [],
    "x_con": [
        (0.0, 1.0)
    ] * 4
}

def toy1(X):
    """
    Toy1 problem with 10-category conditional nonlinear behavior.

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
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]
    x3 = X_con[:, 2]
    x4 = X_con[:, 3]

    base = 2.0

    f = base + np.select(
        [
            cat == "A",
            cat == "B",
            cat == "C",
            cat == "D",
            cat == "E",
            cat == "F",
            cat == "G",
            cat == "H",
            cat == "I",
            cat == "J"
        ],
        [
            np.cos(3.6 * np.pi * (x1 - 2) + x2) + x3 - 1 + x4**2,
            2 * np.cos(1.1 * np.pi * np.exp(x1)) - (x2 / 2) + x3**2 + 2 * np.log(1 + x4**2),
            np.cos(2 * np.pi * x1) + (x2 / 2) + x3 * x4,
            x1 * np.cos(3.4 * np.pi * (x1 - 1)) - x2 - 1 + x3 + x4**3,
            -x1**2 / 2 + np.log(1 + x2**2) + x3**2 + x4,
            2 * (np.cos((np.pi / 4) * np.exp(-x1**4)))**2 - (x2 / 2) + x3 * x4 + 1,
            x1 * np.cos(3.4 * x1) - (x2 / 2) + x3 + x4**3 + 1,
            x1 * (-np.cos(7.0 / (2.0 * np.pi)) * (x2 / 2)) + x3 + x4 + 2,
            -x1**3 / 2 + x2**2 + x3 * x4 + 1,
            -np.cos(5 * np.pi * x1)**2 * np.sqrt(x1) - (-np.log(x2 + x3 + 0.5) / 2) + x4**3 - 1.3
        ],
        default=np.nan  # You can raise error here if desired
    )

    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]
    return f, h, g
