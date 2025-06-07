import numpy as np

def split_into_components(X, bounds):
    """Split unified input X into categorical, integer, and continuous parts using provided bounds."""
    n_cat = len(bounds["x_cat"])
    n_int = len(bounds["x_int"])
    n_con = len(bounds["x_con"])

    X_cat = X[:, :n_cat].astype(str)
    X_int = X[:, n_cat:n_cat + n_int].astype(int)
    X_con = X[:, n_cat + n_int:].astype(float)

    return X_cat, X_int, X_con

def check_bounds(X_cat, X_int, X_con, bounds):
    """Raise ValueError if any variable is out of bounds."""
    for j, categories in enumerate(bounds["x_cat"]):
        if not np.all(np.isin(X_cat[:, j], categories)):
            raise ValueError(f"x_cat[:, {j}] contains invalid categories.")

    for j, (low, high) in enumerate(bounds["x_int"]):
        if not np.all((X_int[:, j] >= low) & (X_int[:, j] <= high)):
            raise ValueError(f"x_int[:, {j}] out of bounds [{low}, {high}].")

    for j, (low, high) in enumerate(bounds["x_con"]):
        if not np.all((X_con[:, j] >= low) & (X_con[:, j] <= high)):
            raise ValueError(f"x_con[:, {j}] out of bounds [{low}, {high}].")