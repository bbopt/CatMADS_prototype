import numpy as np
import sys


# Note that weights for the integer and continuous variables may be set to 1
def general_distance(nb_var, weights, p=2):

    def compute(x, y):
        dist = 0
        for i in range(nb_var):
            dist = dist + weights[i]*abs(x[i]-y[i])**p

        return np.power(dist, 1/p)

    return compute


class IDW:
    def __init__(self, X_train, y_train, distance):
        super(IDW, self).__init__()
        self.X_train = X_train  # Design matrix, where a row is a data
        self.y_train = y_train
        self.nb_points = np.shape(X_train)[0]
        self.distance = distance  # self.distance is a function

    def append_new_train_point(self, X_new, y_new):
        self.X_train = np.vstack(self.X_train, X_new)
        self.y_train = np.append(self.y_train, y_new)
        self.nb_points = self.nb_points + 1

    def predict(self, X):

        distances = np.zeros(self.nb_points)


        # Point-by-point of the training set
        for i in range(self.nb_points):
            # Distance for points (over the variables)
            distances[i] = self.distance(X, self.X_train[i])

        idx_zero_distance = np.where(distances <= 1e-10)
        if np.any(idx_zero_distance):  # Take the average
            return np.mean(self.y_train[idx_zero_distance])
        else:
            weights = np.reciprocal(distances)
            return np.sum(np.multiply(weights, self.y_train))/np.sum(weights)


    def RMSE(self, X_test, y_test):
        nb_test_points = X_test.shape[0]
        squared_sum = 0
        for i in range(nb_test_points):
            squared_sum = squared_sum + ((self.predict(X_test[i]) - y_test[i])**2)/nb_test_points
        return np.sqrt(squared_sum)
