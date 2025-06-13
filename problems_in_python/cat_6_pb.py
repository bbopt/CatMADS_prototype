import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        [0, 1, 2],
        [0, 1, 2]
    ],
    "x_int": [],
    "x_con": [
        (0.0, 100.0),
        (0.0, 100.0)
    ]
}


def cat_6(X):
    """
    Cat-6 based on the Goldstein problem from
        J. Pelamatti, L. Brevault, M. Balesdent, E.-G. Talbi, and Y. Guerin. Efficient global optimization of
        constrained mixed variable problems. Journal of Global Optimization, 73(3):583–613, 2019

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

    cat1 = X_cat[:, 0].astype(int)
    cat2 = X_cat[:, 1].astype(int)
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]

    grid = np.array([20, 50, 80])
    x3 = grid[cat1]  # mapped from cat1
    x4 = grid[cat2]  # mapped from cat2

    # Compute p(...) function
    p = (
        53.3108
        + 0.184901 * x1
        - 5.02914e-6 * x1**3
        + 7.72522e-8 * x1**4
        - 0.0870775 * x2
        - 0.106959 * x3
        + 7.98772e-6 * x3**3
        + 0.00242482 * x4
        + 1.32851e-6 * x4**3
        - 0.00146393 * x1 * x2
        - 0.00301588 * x1 * x3
        - 0.00272291 * x1 * x4
        + 0.0017004  * x2 * x3
        + 0.0038428  * x2 * x4
        - 0.000198969 * x3 * x4
        + 1.86025e-5 * x1 * x2 * x3
        - 1.88719e-6 * x1 * x2 * x4
        + 2.50923e-5 * x1 * x3 * x4
        - 5.62199e-5 * x2 * x3 * x4
    )

    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return p, h, g
