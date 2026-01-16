from setup_and_utils import BASE_PATH, CACHE_PATH, CATDIRECTIONS_PATH, PARAMS_PATH, read_pbinfo_and_cache, split_point
from IDW_model import mixed_distance_with_embeddings, IDW

import numpy as np
import math
import sys
import os
import ctypes
import itertools
import pickle
import time

# TEMPORARY FIX to use PyNomad
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


def categorical_embedding_per_variable(X_cats, weights, nb_categories_per_cat_var):
    weights = np.array(weights)
    embedding_matrix = weights.reshape(-1, 2)

    row_offsets = np.cumsum([0] + nb_categories_per_cat_var[:-1])
    n_samples, nb_cat = X_cats.shape
    embedded = np.zeros((n_samples, 2 * nb_cat))

    for j in range(nb_cat):
        indices = row_offsets[j] + X_cats[:, j]
        # Two parameters per category 
        embedded[:, 2*j : 2*j+2] = embedding_matrix[indices]

    return embedded


def prepare_inputs(weights, X_cats, X_int_normalized, X_con_normalized, nb_categories_per_cat_var):
    X_embedded_cat = categorical_embedding_per_variable(X_cats, weights, nb_categories_per_cat_var)
    return np.hstack([X_embedded_cat, X_int_normalized, X_con_normalized])


if __name__ == '__main__':

    start_time = time.time()

    # Setup 
    K =  3  # number of folds
    budget_per_variable_dist = 50
    data_file_for_cat_dist = os.path.join(BASE_PATH, "..", "Porifera", "solid-fractions-all-l25.txt")


    # Temporary fix
    #seen_last_two = set()
    #X_rows, y_values = [], []
    #with open(data_file_for_cat_dist, 'r') as file:
    #    for line in file:
    #        parts = line.strip().split()
    #
    #        # Extract target value (last column) and check for 'Inf'
    #        try:
    #            target = float(parts[5])
    #            if math.isinf(target):
    #                continue  # skip rows where target is Inf
    #        except ValueError:
    #            continue  # skip rows with non-convertible target
    #
    #        # Extract features (columns 1 to 4)
    #        features = list(map(float, parts[1:5]))
    #
    #        # Get last two variables (columns 3 and 4)
    #        key = (features[2], features[3])  # 0-based: parts[3] and parts[4]
    #
    #        if key in seen_last_two:
    #            continue  # skip duplicates
    #        seen_last_two.add(key)
    #
    #        X_rows.append(features)
    #        y_values.append(target)

    X_rows, y_values = [], []

    with open(data_file_for_cat_dist, 'r') as file:
        for line in file:
            parts = line.strip().split()

            # Extract and validate objective value (last column)
            try:
                target = float(parts[5])
                if math.isinf(target):
                    continue  # Skip rows with Inf in objective
            except ValueError:
                continue  # Skip if the value can't be parsed

            # Extract features (columns 1 to 4)
            features = list(map(float, parts[1:5]))

            # Keep the point
            X_rows.append(features)
            y_values.append(target)

    X_vals = np.array(X_rows)
    f_vals = np.array(y_values)

    # For testing
    n_total = X_vals.shape[0]
    n_sample = 200
    selected_indices = np.random.choice(n_total, size=n_sample, replace=False)
    X_vals = X_vals[selected_indices]
    f_vals = f_vals[selected_indices]

    # Get problem information and data pts
    #(var_types, var_nbs, low_bounds, up_bounds, _, nb_cat_neighbors, X_vals, f_vals, c_vals, 
    #        step, x_frame_center, _, seed_setup, budget_per_variable) = read_pbinfo_and_cache(CACHE_PATH)
    (var_types, var_nbs, low_bounds, up_bounds, _, nb_cat_neighbors, _, _, c_vals, 
        step, x_frame_center, _, seed_setup, budget_per_variable) = read_pbinfo_and_cache(CACHE_PATH)

    # Note that x_frame_center is either primary or secondary, depending on which poll is done in NOMAD
    
    # Fix see for reproducibility and use seed from NOMAD
    np.random.seed(seed_setup)  

    # Dimensions of each variable type
    nb_cat = int(var_nbs[0])
    nb_int = int(var_nbs[1])
    nb_con = int(var_nbs[2])
    nb_var = nb_cat + nb_int + nb_con

    # Retrieve categorical vectors
    X_cats = X_vals[:, :nb_cat].astype(int)

    # Find number of categories per categorical variables
    nb_categories_per_cat_var = []
    for i, var_type in enumerate(var_types):
        if var_type == 'I' and i<nb_cat:
            nb_categories_per_cat_var.append(int(up_bounds[i]+1))

    # Find of categories and parameters with the 2D embedding
    nb_categories = sum(nb_categories_per_cat_var)
    nb_cat_embeddings = 2*nb_categories

    # Normalize continuous and integer variables with bounds in X_vals_onehot
    int_low_bounds = np.array(low_bounds[nb_cat : nb_cat + nb_int])
    int_up_bounds  = np.array(up_bounds[nb_cat : nb_cat + nb_int])
    cont_low_bounds = np.array(low_bounds[nb_cat + nb_int : nb_cat + nb_int + nb_con])
    cont_up_bounds  = np.array(up_bounds[nb_cat + nb_int : nb_cat + nb_int + nb_con])

    X_int_part = X_vals[:, nb_cat : nb_cat + nb_int] if nb_int > 0 else np.empty((X_vals.shape[0], 0))
    X_con_part = X_vals[:, nb_cat + nb_int : nb_cat + nb_int + nb_con] if nb_con > 0 else np.empty((X_vals.shape[0], 0))

    X_int_normalized = normalize(X_int_part, int_low_bounds, int_up_bounds) if nb_int > 0 else X_int_part
    X_con_normalized = normalize(X_con_part, cont_low_bounds, cont_up_bounds) if nb_con > 0 else X_con_part

    # Normalize the outputs
    f_min = np.min(f_vals)
    f_max = np.max(f_vals)  
    f_vals_normalized = (f_vals - f_min) / (f_max - f_min)

    # Create K-folds and associated blackboxes
    nb_data = X_vals.shape[0]
    indices = np.arange(nb_data)
    np.random.shuffle(indices)

    fold_sizes = np.full(K, nb_data // K)
    fold_sizes[:nb_data % K] += 1

    current = 0
    bbs_folds = []
    for k in range(K):
        start, stop = current, current + fold_sizes[k]
        valid_indices = indices[start:stop]
        train_indices = np.concatenate((indices[:start], indices[stop:]))

        # Data passed as default arguments to ensure data is implicitly stored
        def bb_fold(params,
                    X_cats_train=X_cats[train_indices],
                    X_cats_valid=X_cats[valid_indices],
                    X_int_train=X_int_normalized[train_indices],
                    X_int_valid=X_int_normalized[valid_indices],
                    X_con_train=X_con_normalized[train_indices],
                    X_con_valid=X_con_normalized[valid_indices],
                    y_train=f_vals_normalized[train_indices],
                    y_valid=f_vals_normalized[valid_indices]):
            
            nb_weights = nb_cat + nb_int + nb_con
            weight_params = params[:nb_var]
            embedding_params = params[nb_var:]

            # Embed and concatenate
            X_train = prepare_inputs(embedding_params, X_cats_train, X_int_train, X_con_train, nb_categories_per_cat_var)
            X_valid = prepare_inputs(embedding_params, X_cats_valid, X_int_valid, X_con_valid, nb_categories_per_cat_var)

            distance = mixed_distance_with_embeddings(nb_cat, nb_int, nb_con, weight_params, p=2)
            model_IDW = IDW(X_train, y_train, distance)
            return model_IDW.RMSE(X_valid, y_valid)

        bbs_folds.append(bb_fold)
        current = stop

    # Average the many bbs
    def bb(weights):
        sum = 0
        for k in range(K):
            sum = sum + bbs_folds[k](weights)

        return sum/K

    # Define blackbox for PyNomad solver
    def bb_nomad(x):
        try:
            # BB : (theta_u2, theta_u3, w1, w2, w3, w4, w5)
            f = bb([x.get_coord(i) for i in range(x.size())])                             
            x.setBBO(str(f).encode("UTF-8"))
        except:
            print("Unexpected eval error", sys.exc_info()[0])
            return 0
        return 1  # 1: success 0: failed evaluation

    nb_params = nb_var + nb_cat_embeddings 
    budget = budget_per_variable_dist*(nb_params)
    budget_LHS = int(budget * 0.33)
    params = ["BB_OUTPUT_TYPE OBJ", "DISPLAY_DEGREE 2", "DISPLAY_ALL_EVAL false", "DISPLAY_STATS BBE OBJ",
              "MAX_BB_EVAL " + str(budget),
              "LH_SEARCH " + str(budget_LHS) + " 0",
              "SEED " + str(seed_setup),
              "FIXED_VARIABLE " + str(nb_cat) + "-" + str(nb_var - 1)
              ]
    # Weights for continuous and integer variables are fixed : with index starting at 0,
    # the quantitative variables starts at nb_relax_cat

    # Optimize
    lb_weights = [0] * nb_var
    ub_weights = [1] * nb_var
    lb_embeddings = [-1] * nb_cat_embeddings
    ub_embeddings = [1]  * nb_cat_embeddings
    lb = lb_weights + lb_embeddings
    ub = ub_weights + ub_embeddings
    x0_weights = [1.0] * nb_var
    x0_embeddings = np.random.uniform(-1, 1, size=nb_cat_embeddings).tolist()
    x0 = x0_weights + x0_embeddings
    result = PyNomad.optimize(bb_nomad, x0, lb, ub, params)
    params = result['x_best']

    try:
        with open(PARAMS_PATH, "wb") as f:
            pickle.dump(params, f)
        
        end_time = time.time()
        elapsed_time = end_time - start_time
        print(f"Elapsed time: {elapsed_time:.2f} seconds")

        # Ending OK
        sys.exit(0)

    except Exception:
        # Ending not OK: cast error to NOMAD
        sys.exit(1)