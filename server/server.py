import socket
import requests
import threading
import argparse
import logging
import json
import sys

OPCODE_DATA = 1
OPCODE_WAIT = 2
OPCODE_DONE = 3
OPCODE_QUIT = 4

class Server:
    def __init__(self, name, algorithm, dimension, index, port, caddr, cport, ntrain, ntest):
        logging.info("[*] Initializing the server module to receive data from the edge device")
        self.name = name
        self.algorithm = algorithm
        self.dimension = dimension
        self.index = index
        self.caddr = caddr
        self.cport = cport
        self.ntrain = ntrain
        self.ntest = ntest
        success = self.connecter()

        if success:
            self.port = port
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.bind(("0.0.0.0", port))
            self.socket.listen(10)
            self.listener()

    def connecter(self):
        success = True  # Initialize success flag as True
        self.ai = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a new socket object
        self.ai.connect((self.caddr, self.cport))  # Connect to the ai module at the specified address and port
        url = "http://{}:{}/{}".format(self.caddr, self.cport, self.name)  # Format the URL for the request
        request = {}  # Initialize the request dictionary
        request['algorithm'] = self.algorithm  # Set the algorithm of the request
        request['dimension'] = self.dimension  # Set the dimension of the request
        request['index'] = self.index  # Set the index of the request
        js = json.dumps(request)  # Convert the request dictionary to a JSON string
        logging.debug("[*] To be sent to the AI module: {}".format(js))  # Log the JSON string
        result = requests.post(url, json=js)  # Send a POST request to the ai module with the JSON string as the body
        response = json.loads(result.content)  # Parse the response content as JSON
        logging.debug("[*] Received: {}".format(response))  # Log the response

        if "opcode" not in response:  # If the response does not contain an opcode
            logging.debug("[*] Invalid response")  # Log an error message
            success = False  # Set the success flag to False
        else:  # If the response does contain an opcode 
            if response["opcode"] == "failure":  # If the opcode is "failure"
                logging.error("Error happened")  # Log an error message
                if "reason" in response:  # If the response contains a reason
                    logging.error("Reason: {}".format(response["reason"]))  # Log the reason for the failure
                    logging.error("Please try again.")  # Log a message asking the user to try again
                else:  # If the response does not contain a reason
                    logging.error("Reason: unknown. not specified")  # Log a message indicating that the reason for the failure is unknown
                success = False  # Set the success flag to False
            else:  # If the opcode is not "failure"
                assert response["opcode"] == "success"  # Assert that the opcode is "success"
                logging.info("[*] Successfully connected to the AI module")  # Log a success message
        return success  # Return the success flag

    def listener(self):
        logging.info("[*] Server is listening on 0.0.0.0:{}".format(self.port))  # Log that the server is listening on the specified port

        while True:  # Start an infinite loop
            client, info = self.socket.accept()  # Accept a connection from a client
            logging.info("[*] Server accept the connection from {}:{}".format(info[0], info[1]))  # Log the IP address and port of the client(edge)

            client_handle = threading.Thread(target=self.handler, args=(client,))  # Create a thread to handle the client(edge)
            client_handle.start()  # Start the thread

    def send_instance(self, vlst, is_training):
        if is_training:
            url = "http://{}:{}/{}/training".format(self.caddr, self.cport, self.name)
        else:
            url = "http://{}:{}/{}/testing".format(self.caddr, self.cport, self.name)
        data = {}
        data["value"] = vlst
        req = json.dumps(data)
        response = requests.put(url, json=req)
        resp = response.json()

        if "opcode" in resp:
            if resp["opcode"] == "failure":
                logging.error("fail to send the instance to the ai module")

                if "reason" in resp:
                    logging.error(resp["reason"])
                else:
                    logging.error("unknown error")
                sys.exit(1)
        else:
            logging.error("unknown response")
            sys.exit(1)

    def parse_data(self, buf, is_training):
        temp = int.from_bytes(buf[0:1], byteorder="big", signed=True)
        humid = int.from_bytes(buf[1:2], byteorder="big", signed=True)
        power = int.from_bytes(buf[2:4], byteorder="big", signed=True)
        month = int.from_bytes(buf[4:5], byteorder="big", signed=True)

        lst = [temp, humid, power, month]
        logging.info("[temp, humid, power, month] = {}".format(lst))

        self.send_instance(lst, is_training)


    # TODO: You should implement your own protocol in this function
    # The following implementation is just a simple example
    def handler(self, client):
        logging.info("[*] Server starts to process the client's request")  # Log that the server is starting to process the client's request

        ntrain = self.ntrain  # Get the number of training iterations from the server object
        url = "http://{}:{}/{}/training".format(self.caddr, self.cport, self.name)  # Format the URL for the training request

        while True:  # Start loop
            # opcode (1 byte): 
            rbuf = client.recv(1)  # Receive 1 byte from the client
            opcode = int.from_bytes(rbuf, "big")  # Convert the received byte to an integer
            logging.debug("[*] opcode: {}".format(opcode))  # Log the opcode

            if opcode == OPCODE_DATA:  # If the opcode is OPCODE_DATA
                logging.info("[*] data report from the edge")  # Log that data is being reported from the edge
                rbuf = client.recv(5)  # Receive 5 bytes from the client
                logging.debug("[*] received buf: {}".format(rbuf))  # Log the received bytes
                self.parse_data(rbuf, True)  # Parse the received data
            else:  # If the opcode is not OPCODE_DATA
                logging.error("[*] invalid opcode")  # Log that the opcode is invalid
                logging.error("[*] please try again")  # Log a message asking the client to try again
                sys.exit(1)  # Exit the program with an error code

            ntrain -= 1  # Decrement the number of training iterations

            if ntrain > 0:  # If there are more training iterations to be done
                opcode = OPCODE_DONE  # Set the opcode to OPCODE_DONE
                logging.debug("[*] send the opcode OPCODE_DONE")  # Log that the opcode OPCODE_DONE is being sent
                client.send(int.to_bytes(opcode, 1, "big"))  # Send the opcode to the client
            else:  # If there are no more training iterations to be done
                opcode = OPCODE_WAIT  # Set the opcode to OPCODE_WAIT
                logging.debug("[*] send the opcode OPCODE_WAIT")  # Log that the opcode OPCODE_WAIT is being sent
                client.send(int.to_bytes(opcode, 1, "big"))  # Send the opcode to the client
                break  # Break the loop

        result = requests.post(url)  # Send a POST request to the training URL
        response = json.loads(result.content)  # Parse the response content as JSON
        logging.debug("[*] return: {}".format(response["opcode"]))  # Log the opcode in the response

        ntest = self.ntest  # Get the number of testing iterations
        url = "http://{}:{}/{}/testing".format(self.caddr, self.cport, self.name)  # Format the URL for the testing request
        opcode = OPCODE_DONE  # Set the opcode to OPCODE_DONE
        logging.debug("[*] send the opcode OPCODE_DONE")  # Log that the opcode OPCODE_DONE is being sent
        client.send(int.to_bytes(opcode, 1, "big"))  # Send the opcode to the client

        while ntest > 0:  # While there are more testing iterations to be done
            # opcode (1 byte): 
            rbuf = client.recv(1)  # Receive 1 byte from the client
            opcode = int.from_bytes(rbuf, "big")  # Convert the received byte to an integer
            logging.debug("[*] opcode: {}".format(opcode))  # Log the opcode

            if opcode == OPCODE_DATA:  # If the opcode is OPCODE_DATA
                logging.info("[*] data report from the edge")  # Log that data is being reported from the edge
                rbuf = client.recv(5)  # Receive 5 bytes from the client
                logging.debug("[*] received buf: {}".format(rbuf))  # Log the received bytes
                self.parse_data(rbuf, False)  # Parse the received data
            else:  # If the opcode is not OPCODE_DATA
                logging.error("[*] invalid opcode")  # Log that the opcode is invalid
                logging.error("[*] please try again")  # Log a message asking the client to try again
                sys.exit(1)  # Exit the program with an error code

            ntest -= 1  # Decrement the number of testing iterations

            if ntest > 0:  # If there are more testing iterations to be done
                opcode = OPCODE_DONE  # Set the opcode to OPCODE_DONE
                client.send(int.to_bytes(opcode, 1, "big"))  # Send the opcode to the client
            else:  # If there are no more testing iterations to be done
                opcode = OPCODE_QUIT  # Set the opcode to OPCODE_QUIT
                client.send(int.to_bytes(opcode, 1, "big"))  # Send the opcode to the client
                break  # Break the loop

        url = "http://{}:{}/{}/result".format(self.caddr, self.cport, self.name)  # Format the URL for the result request
        result = requests.get(url)  # Send a GET request to the result URL
        response = json.loads(result.content)  # Parse the response content as JSON
        logging.debug("response: {}".format(response))  # Log the response
        if "opcode" not in response:  # If the response does not contain an opcode
            logging.error("invalid response from the AI module: no opcode is specified")  # Log an error message
            logging.error("please try again")  # Log a message asking the client to try again
            sys.exit(1)  # Exit the program with an error code
        else:  # If the response does contain an opcode
            if response["opcode"] == "failure":  # If the opcode is "failure"
                logging.error("getting the result from the AI module failed")  # Log an error message
                if "reason" in response:  # If the response contains a reason
                    logging.error(response["reason"])  # Log the reason for the failure
                logging.error("please try again")  # Log a message asking the client to try again
                sys.exit(1)  # Exit the program with an error code
            elif response["opcode"] == "success":  # If the opcode is "success"
                self.print_result(response)  # Print the result
            else:  # If the opcode is neither "failure" nor "success"
                logging.error("unknown error")  # Log an error message
                logging.error("please try again")  # Log a message asking the client to try again
                sys.exit(1)  # Exit the program with an error code

    def print_result(self, result):
        logging.info("=== Result of Prediction ({}) ===".format(self.name))
        logging.info("   # of instances: {}".format(result["num"]))
        logging.debug("   sequence: {}".format(result["sequence"]))
        logging.debug("   prediction: {}".format(result["prediction"]))
        logging.info("   correct predictions: {}".format(result["correct"]))
        logging.info("   incorrect predictions: {}".format(result["incorrect"]))
        logging.info("   accuracy: {}\%".format(result["accuracy"]))

def command_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--algorithm", metavar="<AI algorithm to be used>", help="AI algorithm to be used", type=str, required=True)
    parser.add_argument("-d", "--dimension", metavar="<Dimension of each instance>", help="Dimension of each instance", type=int, default=1)
    parser.add_argument("-b", "--caddr", metavar="<AI module's IP address>", help="AI module's IP address", type=str, required=True)
    parser.add_argument("-c", "--cport", metavar="<AI module's listening port>", help="AI module's listening port", type=int, required=True)
    parser.add_argument("-p", "--lport", metavar="<server's listening port>", help="Server's listening port", type=int, required=True)
    parser.add_argument("-n", "--name", metavar="<model name>", help="Name of the model", type=str, default="model")
    parser.add_argument("-x", "--ntrain", metavar="<number of instances for training>", help="Number of instances for training", type=int, default=10)
    parser.add_argument("-y", "--ntest", metavar="<number of instances for testing>", help="Number of instances for testing", type=int, default=10)
    parser.add_argument("-z", "--index", metavar="<the index number for the power value>", help="Index number for the power value", type=int, default=0)
    parser.add_argument("-l", "--log", metavar="<log level (DEBUG/INFO/WARNING/ERROR/CRITICAL)>", help="Log level (DEBUG/INFO/WARNING/ERROR/CRITICAL)", type=str, default="INFO")
    args = parser.parse_args()
    return args

def main():
    args = command_line_args()
    logging.basicConfig(level=args.log)

    if args.ntrain <= 0 or args.ntest <= 0:
        logging.error("Number of instances for training or testing should be larger than 0")
        sys.exit(1)

    Server(args.name, args.algorithm, args.dimension, args.index, args.lport, args.caddr, args.cport, args.ntrain, args.ntest)

if __name__ == "__main__":
    main()
