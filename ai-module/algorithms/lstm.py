import sys
import copy
import logging
import numpy as np
from algorithms.algorithm import Algorithm
from keras.models import Sequential
from keras.layers import Dense
from keras.layers import LSTM
from keras.layers import Activation
from keras.layers import Dropout
from keras.layers import Input
from sklearn.metrics import mean_squared_error
from sklearn.preprocessing import StandardScaler

SEQUENCE_LENGTH = 5
THRESHOLD = 0.5

class Lstm(Algorithm):
    def __init__(self, name):
        super().__init__(name)

    # Please implement the following functions
    # Concerning dataset, refer to the class TrainingSet
    def learning(self, dataset, dimension=1):
        training_set = []
        labels = []
        for i in range(len(dataset)-SEQUENCE_LENGTH-1):
            training_set.append(dataset[i:i+SEQUENCE_LENGTH])
            labels.append([dataset[i+SEQUENCE_LENGTH+1]])

        training_set = np.array(training_set)
        labels = np.array(labels)

        logging.debug("training_set: {}".format(training_set))
        logging.debug("training_set.shape: {}".format(training_set.shape))
        logging.debug("labels.shape: {}".format(labels.shape))

        self.predictor = Sequential()
        self.predictor.add(LSTM(128, return_sequences=True, activation='relu', input_shape=(training_set.shape[1], training_set.shape[2])))
        self.predictor.add(Dropout(0.5))
        self.predictor.add(LSTM(128, return_sequences=True, activation='relu'))
        self.predictor.add(Dropout(0.5))
        self.predictor.add(Dense(dimension))
        self.predictor.compile(loss='mean_squared_error', optimizer='adam')

        try:
            self.predictor.fit(training_set, labels, epochs=50, verbose=1)
            logging.info("The {} predictor is well generated".format(self.get_name()))
        except:
            self.predictor = None
            logging.info("The {} predictor is not generated".format(self.get_name()))

        return self.predictor != None

    def prediction(self, value, dimension=1):
        if len(self.queue) >= SEQUENCE_LENGTH:
            self.queue.pop(0)
        self.queue.append(value)
        if len(self.queue) < SEQUENCE_LENGTH:
            ret = []
            for _ in range(dimension):
                ret.append(-1)
            return ret
        sequence = np.array([self.queue])
        #sequence = sequence.reshape((sequence.shape[0], 1, sequence.shape[1]))
        #pred = list(self.predictor.predict(sequence))[0][0][0]
        pred = list(self.predictor.predict(sequence))[0][0]
        logging.debug("pred in algorithm: {}".format(pred))

        return pred
