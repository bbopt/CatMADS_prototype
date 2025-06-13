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


def cat_3(X):
    """
    Cat-3 based on the Branin problem.

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

    cat1 = X_cat[:, 0]
    cat2 = X_cat[:, 1]
    int1 = X_int[:, 0]
    int2 = X_int[:, 1]
    con1 = X_con[:, 0]
    con2 = X_con[:, 1]

    # Vectorized h(...)
    scaled_x = 15 * con1
    term1 = 15 * con2 - (int1 / (4 * np.pi**2)) * ((scaled_x - int2) ** 2)
    term2 = (int1 / np.pi) * (scaled_x - int2)
    term3 = (int1 - (1 / (8 * np.pi))) * np.cos(scaled_x - 5) + int1
    h_val = ((term1 + term2) ** 2 + term3 - 54.8104) / 51.9496

    # Compute the categorical logic weights
    f = np.select(
        [ (cat1 == "A") & (cat2 == "A"),
          (cat1 == "A") & (cat2 == "B"),
          (cat1 == "B") & (cat2 == "A"),
          (cat1 == "B") & (cat2 == "B") ],
        [ h_val,
          0.4 * h_val,
         -0.75 * h_val,
         -0.5 * h_val ]
    )

    f += 10
    h = np.zeros(X.shape[0])
    g = [[] for _ in range(X.shape[0])]

    return f, h, g
