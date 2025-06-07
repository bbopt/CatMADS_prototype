import numpy as np
from _utils import split_into_components, check_bounds

# Define BOUNDS / METADATA
BOUNDS = {
    "x_cat": [
        ["A", "B"],
        ["A", "B"]
    ],
    "x_int": [
        (1, 5),
        (1, 5)
    ],
    "x_con": [
        (0.0, 1.0),
        (0.0, 1.0)
    ]
}

def branin_constrained(X):
    """
    Constrained Branin problem with categorical-conditioned objective scaling and constraint.

    Parameters:
        X : ndarray of shape (n_samples, n_variables)

    Returns:
        f : ndarray of shape (n_samples,)
        h : ndarray of shape (n_samples,) — aggregated constraint violation
        g : list of lists — individual constraint values per sample
    """
    X = np.atleast_2d(X)
    X_cat, X_int, X_con = split_into_components(X, BOUNDS)
    check_bounds(X_cat, X_int, X_con, BOUNDS)

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    i1 = X_int[:, 0]
    i2 = X_int[:, 1]
    x1 = X_con[:, 0]
    x2 = X_con[:, 1]

    # Normalized Branin function
    def h_branin(i1, i2, x1, x2):
        term1 = 15 * x2 - (i1 / (4 * np.pi**2)) * ((15 * x1 - i2) ** 2)
        term2 = (i1 / np.pi) * (15 * x1 - i2)
        term3 = (i1 - 1 / (8 * np.pi)) * np.cos(15 * x1 - 5) + i1
        return ((term1 + term2) ** 2 + term3 - 54.8104) / 51.9496

    h_raw = h_branin(i1, i2, x1, x2)

    f = np.select(
        [
            (cat1 == "A") & (cat2 == "A"),
            (cat1 == "A") & (cat2 == "B"),
            (cat1 == "B") & (cat2 == "A"),
            (cat1 == "B") & (cat2 == "B"),
        ],
        [
            h_raw,
            0.4 * h_raw,
            -0.75 * h_raw,
            -0.5 * h_raw
        ],
        default=np.nan
    )

    g1 = np.select(
        [
            (cat1 == "A") & (cat2 == "A"),
            (cat1 == "A") & (cat2 == "B"),
            (cat1 == "B") & (cat2 == "A"),
            (cat1 == "B") & (cat2 == "B")
        ],
        [
            -x1 * x2 + (i1 / 10.0),
            -1.5 * x1 * x2 + (i2 / 10.0),
            -1.5 * x1 * x2 + (i1 / 5.0),
            -1.2 * x1 * x2 + (i2 / 7.5)
        ],
        default=np.nan
    )

    g = [[g1[k]] for k in range(X.shape[0])]
    h = np.array([max(0, gk[0])**2 for gk in g])

    return f, h, g
