import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B", "C"],
        ["A", "B", "C"]
    ],
    "x_int": [],
    "x_con": [
        (0.0, 100.0),
        (0.0, 100.0)
    ]
}


def cat_cstrs_7(X):
    """
    Cat-cstrs-7 based on the Goldstein problem from
        J. Pelamatti, L. Brevault, M. Balesdent, E.-G. Talbi, and Y. Guerin. Efficient global optimization of
        constrained mixed variable problems. Journal of Global Optimization, 73(3):583–613, 2019

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,) — aggregated constraint violation
        g : list of lists — individual constraint value per sample
    """
    X = np.atleast_2d(X)
    X_cat, _, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, np.empty((X.shape[0], 0)), X_con, BOUNDS)

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    x1, x2 = X_con.T

    # Table-based values for x3 and x4
    table = {
        ("A", "A"): (20, 20), ("A", "B"): (50, 20), ("A", "C"): (80, 20),
        ("B", "A"): (20, 50), ("B", "B"): (50, 50), ("B", "C"): (80, 50),
        ("C", "A"): (20, 80), ("C", "B"): (50, 80), ("C", "C"): (80, 80),
    }
    keys = list(zip(cat1, cat2))
    x3 = np.array([table[k][0] for k in keys])
    x4 = np.array([table[k][1] for k in keys])

    # Objective function (p)
    f = (
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
        + 0.0017004 * x2 * x3
        + 0.0038428 * x2 * x4
        - 0.000198969 * x3 * x4
        + 1.86025e-5 * x1 * x2 * x3
        - 1.88719e-6 * x1 * x2 * x4
        + 2.50923e-5 * x1 * x3 * x4
        - 5.62199e-5 * x2 * x3 * x4
    )

    # Constraint
    g1 = x3 * (np.sin(x1 / 100) ** 3) + x4 * (np.sin(x2 / 10) ** 3)
    g = [[g1[k]] for k in range(X.shape[0])]
    h = np.array([max(0, gk[0])**2 for gk in g])

    return f, h, g
