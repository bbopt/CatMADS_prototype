import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["absolute", "quad", "logsum", "hyperbol", "invgauss"]
    ],
    "x_int": [
        (-5, 5)
    ] * 5,
    "x_con": [
        (-5.0, 5.0)
    ] * 5
}


def cat_12(X):
    """
    Cat-12 Based on the Styblinski-Tang problem.

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
    f = np.zeros(X.shape[0])

    # Vectorized loop over the 5 dimensions
    for i in range(5):
        x_i = X_con[:, i]
        z_i = X_int[:, i]

        p_val = np.select(
            [cat == "absolute", cat == "quad", cat == "logsum", cat == "hyperbol", cat == "invgauss"],
            [
                np.abs(x_i - 1),
                (x_i**2 + 1) / 2.0,
                np.exp(np.abs(x_i + 1)) - 1,
                (x_i**2) / (1 + np.abs(x_i)),
                1 - np.exp(-x_i**2)
            ],
            default=0.0
        )

        f += 0.5 * (x_i**4 - 16 * x_i**2 + 5 * z_i + 8 * p_val)

    f += 100
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]
    return f, h, g
