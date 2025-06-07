from setup_and_utils import CACHE_PATH, CATDIRECTIONS_PATH, PARAMS_PATH, read_pbinfo_and_cache, split_point
from IDW_model import general_distance, IDW

import numpy as np
import math
import sys
import os
import ctypes
import itertools
import pickle

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

if __name__ == '__main__':

    # Get problem information and data pts
    (var_types, var_nbs, low_bounds, up_bounds, _, nb_cat_neighbors, X_vals, f_vals, c_vals, 
            step, x_frame_center, _, seed_setup, budget_per_variable) = read_pbinfo_and_cache(CACHE_PATH)

    # Note that x_frame_center is either primary or secondary, depending on which poll is done in NOMAD

    # Choose number of folds
    K =  3
    np.random.seed(seed_setup)

    # Dimensions of each variable type
    nb_cat = int(var_nbs[0])
    nb_int = int(var_nbs[1])
    nb_con = int(var_nbs[2])

    # Find number of categories per categorical variables
    nb_categories_per_cat_var = []
    for i, var_type in enumerate(var_types):
        if var_type == 'I' and i<nb_cat:
            nb_categories_per_cat_var.append(int(up_bounds[i]+1))

    # Find number of binary variables in relaxed subp
    nb_relax_cat = sum(nb_categories_per_cat_var)

    # Transform X_vals into one-hot encoding: the first nb_cat variables are categorical  
    # The i-th categorical variables is label encoded with 1,2,...,nb_categories_per_cat_var[i]
    X_onehot_list = []

    cat_idx = 0  
    for i in range(nb_cat):
        nb_cat_i = nb_categories_per_cat_var[cat_idx]
        col = X_vals[:, i].astype(int)
        onehot = np.zeros((X_vals.shape[0], nb_cat_i))
        onehot[np.arange(X_vals.shape[0]), col] = 1
        X_onehot_list.append(onehot)
        cat_idx += 1

    # Concatenate all one-hot parts
    X_cat_onehot = np.hstack(X_onehot_list)

    # Bounds of the subpb
    # Categorical relaxed one-hot variables
    bounds = [(0, 1)] * nb_relax_cat 
    # Integer relaxed
    bounds += [(low_bounds[i], up_bounds[i]) for i, var_type in enumerate(var_types) if (var_type == 'I' and nb_cat<=i<(nb_cat+nb_int))] 
    # Continuous
    bounds += [(low_bounds[i], up_bounds[i]) for i, var_type in enumerate(var_types) if var_type == 'R'] 

    # Normalize continuous and integer variables with bounds in X_vals_onehot
    int_low_bounds = np.array(low_bounds[nb_cat : nb_cat + nb_int])
    int_up_bounds  = np.array(up_bounds[nb_cat : nb_cat + nb_int])
    cont_low_bounds = np.array(low_bounds[nb_cat + nb_int : nb_cat + nb_int + nb_con])
    cont_up_bounds  = np.array(up_bounds[nb_cat + nb_int : nb_cat + nb_int + nb_con])

    X_int_part = X_vals[:, nb_cat : nb_cat + nb_int] if nb_int > 0 else np.empty((X_vals.shape[0], 0))
    X_con_part = X_vals[:, nb_cat + nb_int : nb_cat + nb_int + nb_con] if nb_con > 0 else np.empty((X_vals.shape[0], 0))

    X_int_normalized = normalize(X_int_part, int_low_bounds, int_up_bounds) if nb_int > 0 else X_int_part
    X_con_normalized = normalize(X_con_part, cont_low_bounds, cont_up_bounds) if nb_con > 0 else X_con_part

    X_vals_onehot_normalized = np.hstack([X_cat_onehot, X_int_normalized, X_con_normalized])

    # Normalize the outputs
    f_min = np.min(f_vals)
    f_max = np.max(f_vals)
    f_vals_normalized = (f_vals - f_min) / (f_max - f_min)
    # f_original = f_vals_normalized * (f_max - f_min) + f_min

    # Create K-folds with data
    nb_data = X_vals_onehot_normalized.shape[0]
    indices = np.arange(nb_data)
    np.random.shuffle(indices)

    X_train_onehot_normalized_Kfolds = {k: None for k in range(K)}
    y_train_normalized_Kfolds = {k: None for k in range(K)}
    X_valid_onehot_normalized_Kfolds = {k: None for k in range(K)}
    y_valid_normalized_Kfolds = {k: None for k in range(K)}

    # Compute fold sizes
    fold_sizes = np.full(K, nb_data // K)
    fold_sizes[:nb_data % K] += 1

    current = 0
    for k in range(K):
        start, stop = current, current + fold_sizes[k]
        valid_indices = indices[start:stop]
        train_indices = np.concatenate((indices[:start], indices[stop:]))

        X_train_onehot_normalized_Kfolds[k] = X_vals_onehot_normalized[train_indices]
        y_train_normalized_Kfolds[k] = f_vals_normalized[train_indices]

        X_valid_onehot_normalized_Kfolds[k] = X_vals_onehot_normalized[valid_indices]
        y_valid_normalized_Kfolds[k] = f_vals_normalized[valid_indices]

        current = stop

    # Create blackboxes parametrized by the training sets
    bbs_folds = []
    for k in range(K):
        
        def bb_fold(weights, k=k):
            # Create distance (nb_var, weights, p=2):
            distance = general_distance(nb_relax_cat + nb_int + nb_con, weights, p=2)

            model_IDW = IDW(X_train_onehot_normalized_Kfolds[k], y_train_normalized_Kfolds[k], distance)
            return model_IDW.RMSE(X_valid_onehot_normalized_Kfolds[k], y_valid_normalized_Kfolds[k])

        # Append blackbox
        bbs_folds.append(bb_fold)


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


    budget = 100*(nb_relax_cat + nb_int + nb_con)
    budget_LHS = int(budget * 0.33)
    params = ["BB_OUTPUT_TYPE OBJ", "DISPLAY_DEGREE 0", "DISPLAY_ALL_EVAL false", "DISPLAY_STATS BBE OBJ",
              "MAX_BB_EVAL " + str(budget),
              "LH_SEARCH " + str(budget_LHS) + " 0",
              "SEED " + str(seed_setup),
              "FIXED_VARIABLE " + str(nb_relax_cat) + "-" + str(nb_relax_cat + nb_int + nb_con - 1)
              ]


    # Optimize
    nb_var_pb = nb_relax_cat + nb_int + nb_con
    #lb = [b[0] for b in bounds]
    #ub = [b[1] for b in bounds]
    lb = [0]*nb_var_pb
    ub = [1]*nb_var_pb
    x0 = np.ones(nb_var_pb)
    result = PyNomad.optimize(bb_nomad, x0, lb, ub, params)
    params = result['x_best']

    with open(PARAMS_PATH, "wb") as f:
        pickle.dump(params, f)


    # Stoppig criterion based on proportion of evaluations
    prop_budget = len(X_vals)/(budget_per_variable*(nb_cat + nb_int + nb_con))
    if prop_budget >= 0.20:
        # Uncomment for debug
        print("\n", ">Max hit budget for update categorical distance.", "\n")
        sys.exit(1)
    else:
        sys.exit(0)
        