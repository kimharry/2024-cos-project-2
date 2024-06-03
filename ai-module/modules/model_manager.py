import argparse
import logging
import sys
from putils.autils import init_algorithms

class ModelManager:
    def __init__(self, algorithm, dimension=1):
        logging.info("Initializing the model manager with {}".format(algorithm))
        self.algorithms = {}
        self.initialized = False
        self.error = False
        self.dimension = dimension
        init_algorithms(self)

        if algorithm not in self.algorithms:
            logging.error("Algorithm {} is not supported".format(algorithm))
            logging.error("Please try again with one of the supported algorithms")
            logging.error("Supported algorithms ===")
            algos = sorted(self.algorithms.keys()) 
            for algo in algos:
                logging.error("  {}".format(algo))
            self.error = True

        if not self.error:
            self.algorithm = algorithm
            logging.info("The model manager is successfully initialized with {}".format(self.algorithm))

    def add_algorithm(self, algorithm):
        self.algorithms[algorithm.get_name()] = algorithm
        logging.debug("Algorithm {} is loaded".format(algorithm.get_name()))

    def set_algorithm(self, algorithm):
        self.algorithm = algorithm

    def get_algorithm(self):
        return self.algorithm

    def get_error_status(self):
        return self.error

    def learning(self, dm, dimension=1):
        return self.algorithms[self.algorithm].learning(dm.get_data(), dimension)

    def prediction(self, value, dimension=1):
        pred = self.algorithms[self.algorithm].prediction(value, dimension)
        return pred

def command_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--algorithm", required=True, metavar="<ml algorithm for prediction>", help="ML algorithm for prediction", type=str)
    parser.add_argument("-l", "--log", metavar="<log level (DEBUG/INFO/WARNING/ERROR/CRITICAL)>", help="Log level (DEBUG/INFO/WARNING/ERROR/CRITICAL)", default="INFO", type=str)
    args = parser.parse_args()
    return args

def main():
    args = command_line_args()
    logging.basicConfig(level=args.log)

    mm = ModelManager(args.algorithm)

if __name__ == "__main__":
    main()
