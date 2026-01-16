from setup_and_utils import CACHE_PATH, CATDIRECTIONS_PATH, PARAMS_PATH, read_pbinfo_and_cache, split_point
from IDW_model import general_distance, IDW

import numpy as np
import math
import sys
import os
import ctypes
import itertools
import pickle

from smt.applications.mixed_integer import MixedIntegerKrigingModel
from smt.surrogate_models import KRG, MixIntKernelType
from smt.design_space import (CategoricalVariable, DesignSpace, FloatVariable, IntegerVariable)


libgomp_path = "/usr/lib/x86_64-linux-gnu/libgomp.so.1"
ctypes.CDLL(libgomp_path, mode=ctypes.RTLD_GLOBAL)
import PyNomad


# Normalize integer and continuous values
def normalize(X, low_bounds, up_bounds):
    return (X - low_bounds) / (up_bounds - low_bounds)


# One-hot encode the categorical combinations
def one_hot_encode(combinations, nb_categories_per_cat_var):
    encoded = []
    for cat_vec in combinations:
        onehot = []
        for val, m_j in zip(cat_vec, nb_categories_per_cat_var):
            vec = [0] * m_j
            vec[val] = 1
            onehot.extend(vec)
        encoded.append(onehot)
    return np.array(encoded)

if __name__ == '__main__':

    # Get problem information and data pts
    (var_types, var_nbs, low_bounds, up_bounds, _, nb_cat_neighbors, X_vals, f_vals, c_vals, 
            step, x_frame_center, _, _, _) = read_pbinfo_and_cache(CACHE_PATH)

    # Dimensions of each variable type
    nb_cat = int(var_nbs[0])
    nb_int = int(var_nbs[1])
    nb_con = int(var_nbs[2])

    # Load the params
    models = None
    with open(PARAMS_PATH, "rb") as f:
        params = pickle.load(f)

    # Find number of categories per categorical variables
    nb_categories_per_cat_var = []
    for i, var_type in enumerate(var_types):
        if var_type == 'I' and i<nb_cat:
            nb_categories_per_cat_var.append(int(up_bounds[i]+1))


    # Find number of binary variables in relaxed subp
    nb_relax_cat = sum(nb_categories_per_cat_var)


    # Generate all possible combinations of categorical vectors
    all_combinations = list(itertools.product(*[range(m_j) for m_j in nb_categories_per_cat_var]))

    # Wrap each combination in a list, so it's in [[x1, x2, ..., xn]] format
    wrapped_combinations = [list(c) for c in all_combinations]
    onehot_wrapped_combinations = np.array(one_hot_encode(wrapped_combinations, nb_categories_per_cat_var))

    # Split current point into components and one-hot encoded xcat
    x_cat, x_int, x_con = split_point(x_frame_center, var_types, nb_cat, nb_int, nb_con)    
    x_cat_onehot = one_hot_encode([x_cat], nb_categories_per_cat_var)

    # Compare x_cat_onehot vs onehot_wrapped_combinations
    params_cat_onehot = np.array(params[:nb_relax_cat])

    # Compute weighted distances with params_cat_onehot
    # x_cat_onehot is compared with all onehot_wrapped_combinations
    # Create a list of distances (index with onehot_wrapped_combinations)
    x_cat_onehot = np.array(x_cat_onehot[0])

    diffs = onehot_wrapped_combinations - x_cat_onehot  # shape = (n_combinations, nb_relax_cat)
    squared_weighted_diffs = (diffs ** 2) * params_cat_onehot  # weight each dimension
    distances = np.sqrt(np.sum(squared_weighted_diffs, axis=1))  # shape: (n_combinations,)


    # Categorical vector to index
    index_x_cat = wrapped_combinations.index(x_cat.tolist())
    # Set the distance to itself to infinity to ignore it
    distances[index_x_cat] = np.inf

    # Find closest categorical components
    closest_indices = np.argsort(distances)[:nb_cat_neighbors]
    closest_vectors = [wrapped_combinations[i] for i in closest_indices]

    # Create new points to suggestion
    x_quant =  np.concatenate((x_int, x_con))
    new_suggestions = []
    for i in range(nb_cat_neighbors):
        new_suggestion = np.concatenate((closest_vectors[i], x_quant)).astype(float)
        #print("Point ", str(i), ": ", new_suggestion)
        new_suggestions.append(new_suggestion)
    new_suggestions = np.array(new_suggestions)


    # Create directions to "get" to new points
    with open(CATDIRECTIONS_PATH, 'w') as file:
        for solution in new_suggestions:
            # Calculate direction as solution - x_frame_center
            direction = [val - x_frame_center[i] for i, val in enumerate(solution)]

            # Format direction values
            formatted_direction = []
            for i, val in enumerate(direction):
                if var_types[i] == 'I':  # Format integer variables as integers
                    formatted_direction.append(f"{int(val)}")
                else:  # Format others as floats
                    formatted_direction.append(f"{val:.6f}")

            # Write formatted direction to file
            file.write(f"({' '.join(formatted_direction)})\n")

