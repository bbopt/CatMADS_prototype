import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        list(range(10))
    ],
    "x_int": [],
    "x_con": [
        (0.0, 1.0)
    ] * 8
}

def toy2(X):
    """
    Toy2 problem with 10-category logic over 8 continuous variables.

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

    cat = X_cat[:, 0].astype(int)
    x = [X_con[:, i] for i in range(8)]  # x[0] to x[7]

    f = np.select(
        [
            cat == 0,
            cat == 1,
            cat == 2,
            cat == 3,
            cat == 4,
            cat == 5,
            cat == 6,
            cat == 7,
            cat == 8,
            cat == 9
        ],
        [
            np.cos(3.6 * np.pi * (x[0] + x[1] - 2)) + np.abs(x[2]) + np.floor(x[3]) - 0.5,
            2 * np.cos(1.1 * np.pi * np.exp(x[0] + x[4])) - np.abs(x[1] + x[5]) / 2 + np.abs(x[2] - x[3]) + 2,
            np.cos(2 * np.pi * (x[0] + x[1] + x[2])) + np.abs(x[3] + x[4]) / 2 - np.floor(x[5]),
            x[0] * x[1] * (np.cos(3.4 * np.pi * (x[2] - 1)) - np.abs(x[3] + x[4]) / 2),
            -np.abs(x[0] * x[5])**2 / 2 + x[2] + np.abs(x[3] - x[4]),
            2 * (np.cos((np.pi / 4) * np.exp(-(x[2] * x[4])**4)))**2 - (x[5] + x[6]) / 2 + np.abs(x[7]) + 1,
            x[1] * np.cos(3.4 * np.pi * (x[3] + x[4])) - x[5] / 2 + np.abs(x[0] - x[6]) + 0.25,
            x[0] * x[7] * (-np.cos((7 * np.pi / 2) * (x[1] + x[2])) - x[5] / 2 + 2),
            -np.abs(x[0] * x[1] * x[2])**3 / 2 + np.abs(x[3]) + np.abs(x[4]),
            -np.cos(5 * np.pi * (x[0] * x[1] + x[2]))**2 * np.sqrt(np.abs(x[3])) -
            np.log(np.abs(x[4]) + 0.5) / 2 + np.abs(x[5]) - 0.6
        ],
        default=np.nan
    )

    f += 2.0
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]
    return f, h, g
