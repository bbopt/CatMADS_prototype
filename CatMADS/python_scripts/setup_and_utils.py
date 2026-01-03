import os
# Path to this script
current_file = os.path.abspath(__file__)

# Parent directory of the script's directory
BASE_PATH = os.path.dirname(os.path.dirname(current_file))


# Files to read
CACHE_PATH = os.path.join(BASE_PATH, "readwrite_files/cachePts.txt")
PTS_TO_EVAL_SURROGATE_PATH = os.path.join(BASE_PATH, "readwrite_files/ptsToSurrogateEval.txt")

# Files to write
CATDIRECTIONS_PATH = os.path.join(BASE_PATH, "readwrite_files/catDirections.txt")
PARAMS_PATH = os.path.join(BASE_PATH, "readwrite_files/params.pkl")


# Packages and functions from packages
import numpy as np


def read_pbinfo_and_cache(file_path):
    variable_types = []
    lower_bounds = []
    upper_bounds = []
    points = []
    values = []
    constraints = []
    best_fct_vals = []

    current_step = None
    current_frame_feasible = None
    current_frame_infeasible = None

    with open(file_path, 'r') as file:

        # First line: variable types
        variable_types_line = file.readline().strip()
        variable_types = variable_types_line.split(":")[1].strip().split()

        # Second line: number of variables per types
        variable_numbers_line = file.readline().strip()
        variable_numbers = variable_numbers_line.split(":")[1].strip().split()
        variable_numbers = [int(x) for x in variable_numbers]

        # Third line: lower bounds
        lower_bounds_line = file.readline().strip()
        lower_bounds = np.array(list(map(float, lower_bounds_line.split(":")[1].strip().split())))

        # Fourth line: upper bounds
        upper_bounds_line = file.readline().strip()
        upper_bounds = np.array(list(map(float, upper_bounds_line.split(":")[1].strip().split())))

        # Fifth line: current step
        current_step_line = file.readline().strip()
        current_step = current_step_line.split(":")[1].strip()

        # Sixth line: current frame (feasible)
        current_frame_feasible_line = file.readline().strip()
        current_frame_feasible_str = current_frame_feasible_line.split(":")[1].strip().strip('()').split()
        if all(item == "-" for item in current_frame_feasible_str):
            current_frame_feasible = "undefined"
        else:
            current_frame_feasible = np.array(list(map(float, current_frame_feasible_str)))

        # Seventh line: current frame (infeasible)
        current_frame_infeasible_line = file.readline().strip()
        current_frame_infeasible_str = current_frame_infeasible_line.split(":")[1].strip().strip('()').split()
        if all(item == "-" for item in current_frame_infeasible_str):
            current_frame_infeasible = "undefined"
        else:
            current_frame_infeasible = np.array(list(map(float, current_frame_infeasible_str)))        

        # Eighth line: best function values of the current solution
        best_fct_vals_line = file.readline().strip()
        best_fct_vals_str = best_fct_vals_line.split(":")[1].strip().split()
        best_fct_vals = np.array(list(map(float, best_fct_vals_str)))

        # Ninth line: nb of categorical neighbors
        nb_cat_neighbors_line = file.readline().strip()
        nb_cat_neighbors_str = nb_cat_neighbors_line.split(":")[1].strip().strip('()').split()
        nb_cat_neighbors = int(nb_cat_neighbors_str[0])

        # --------- new --------- #
        # Tenth line: seed used
        seed_line = file.readline().strip()
        seed_str = seed_line.split(":")[1].strip().strip('()').split()
        seed = int(seed_str[0])

        # Eleventh line: budget per variables 
        budget_per_variable_line = file.readline().strip()
        budget_per_variable_line_str = budget_per_variable_line.split(":")[1].strip().strip('()').split()
        budget_per_variable = int(budget_per_variable_line_str[0])
        # --------- new --------- #


        # Remaining lines: Points and values
        for line in file:
            if 'BB_EVAL_OK' in line:
                # Extract the point values (first set of parentheses)
                point_str = line.split(')')[0].strip('(').strip()
                point_values = list(map(float, point_str.split()))
                points.append(point_values)

                # Extract the function values (second set of parentheses)
                values_str = line.split('BB_EVAL_OK')[1].split('(')[1].split(')')[0].strip().split()
                values_float = list(map(float, values_str))
                
                # The first value is the objective function value
                values.append(values_float[0])
                
                # The rest are the constraint function values
                if len(values_float) > 1:
                    constraints.append(values_float[1:])
                else:
                    constraints.append([])

    points_array = np.array(points)
    values_array = np.array(values)
    constraints_array = np.array([np.array(c) for c in constraints], dtype=float)

    return (variable_types, variable_numbers, lower_bounds, upper_bounds, best_fct_vals, 
            nb_cat_neighbors, points_array, values_array, constraints_array, 
            current_step, current_frame_feasible, current_frame_infeasible, seed, budget_per_variable)



def split_point(point, variable_types, nb_cat, nb_int, nb_con):
    # Separate the values of point based on the type of variable
    categorical_values = []
    integer_values = []
    real_values = []

    for i, var_type in enumerate(variable_types):
        if var_type == 'I' and i<nb_cat:
            categorical_values.append(int(point[i]))
        elif var_type == 'I' and nb_cat <= i < (nb_cat+nb_int):
            integer_values.append(int(point[i]))
        elif var_type == 'R':
            real_values.append(point[i])

    # Convert lists to NumPy arrays (optional, if you need NumPy arrays)
    categorical_values = np.array(categorical_values)
    integer_values = np.array(integer_values)
    real_values = np.array(real_values)

    return categorical_values, integer_values, real_values
