import numpy as np
from _utils import split_into_components, check_bounds

# Optional: number of continuous variables (this can be modified)
n_con = 4

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C"],
        ["positive", "negative", "neutral"]
    ],
    "x_int": [
        (1, 10),
        (-1, 1)
    ],
    "x_con": [
        (-5.0, 5.0)
    ] * n_con
}


def cat_1(X):
    """
    Cat-1 problem based on the Ackley problem.

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,) — zero for unconstrained problems
        g : ndarray of shape (n_samples, 0) — empty for unconstrained problems
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    int1 = X_int[:, 0][:, None]
    int2 = X_int[:, 1][:, None]
    n_con = X_con.shape[1]  # Ensures normalization works even if BOUNDS changes

    sum_con = np.sum(X_con, axis=1)
    penalty = (
        15 * np.abs(sum_con) * (X_cat[:, 1] == "positive")
        + 15 * np.abs(sum_con - 1) * (X_cat[:, 1] == "negative")
        + 10 * np.abs(sum_con) * (X_cat[:, 1] == "neutral")
    )

    f = np.copy(penalty)
    mask_A = X_cat[:, 0] == "A"
    mask_B = X_cat[:, 0] == "B"
    mask_C = X_cat[:, 0] == "C"

    if np.any(mask_A):
        con = X_con[mask_A]
        i1 = int1[mask_A]
        i2 = int2[mask_A]
        f[mask_A] += -20 * np.exp(-0.2 * np.sqrt(np.sum((con + i1)**2, axis=1) / n_con))
        f[mask_A] += -np.exp(np.sum(np.cos(2 * np.pi * con * i2), axis=1) / n_con)

    if np.any(mask_B):
        con = X_con[mask_B]
        i1 = int1[mask_B]
        i2 = int2[mask_B]
        f[mask_B] += -10 * np.exp(-0.1 * np.sqrt(np.sum(np.abs(con + i1)**2, axis=1) / n_con))
        f[mask_B] += 5 * np.sum(np.cos(2 * np.pi * con - i2), axis=1)

    if np.any(mask_C):
        con = X_con[mask_C]
        i1 = int1[mask_C]
        i2 = int2[mask_C]
        f[mask_C] += -15 * np.exp(-np.sqrt(np.sum(np.abs(con * i1), axis=1) / n_con))
        f[mask_C] += -5 * np.exp(-np.sum(np.cos(2 * np.pi * con + i2), axis=1) / n_con)

    f += 50

    # Unconstrained problem
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return f, h, g
