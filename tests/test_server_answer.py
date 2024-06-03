import socket
import threading
import argparse
import logging

OPCODE_REPLY = 2

def protocol_execution(sock):
    # 1. Alice -> Bob: length of the name (4 bytes) || name (length bytes)
    # Get the length information (4 bytes)
    buf = sock.recv(4)  # Receive 4 bytes from the socket
    length = int.from_bytes(buf, "big")  # Convert the bytes to an integer (big-endian)
    logging.info("[*] Length received: {}".format(length))  # Log the received length

    # Get the name (Alice)
    buf = sock.recv(length)  # Receive 'length' bytes from the socket
    logging.info("[*] Name received: {}".format(buf.decode()))  # Decode the bytes to string

    # 2. Bob -> Alice: length of the name (4 bytes) || name (length bytes)
    # Send the length information (4 bytes)
    name = "Bob"  # The name to be sent
    length = len(name)  # Calculate the length of the name
    logging.info("[*] Length to be sent: {}".format(length))  # Log the length to be sent
    sock.send(int.to_bytes(length, 4, "big"))  # Convert the length to bytes and send

    # Send the name (Bob)
    logging.info("[*] Name to be sent: {}".format(name))  # Log the name to be sent
    sock.send(name.encode())  # Convert the name to bytes and send

    # Implement following the instructions below
    # 3. Alice -> Bob: opcode (4 bytes) || arg1 (4 bytes) || arg2 (4 bytes)
    # The opcode should be 1
    buf = sock.recv(12)  # Receive 12 bytes from the socket

    # The values are encoded in the big-endian style and should be translated into the little-endian style (because my machine follows the little-endian style)
    opcode = int.from_bytes(buf[0:4], "little")  # Extract the opcode from the received bytes
    arg1 = int.from_bytes(buf[4:8], "little")  # Extract the first argument from the received bytes
    arg2 = int.from_bytes(buf[8:12], "little")  # Extract the second argument from the received bytes

    logging.info("[*] Opcode: {}".format(opcode))  # Log the opcode
    logging.info("[*] Arg1: {}".format(arg1))  # Log the first argument
    logging.info("[*] Arg2: {}".format(arg2))  # Log the second argument

    # 4. Bob -> Alice: opcode (4 bytes) || result (4 bytes)
    # The opcode should be 2
    result = arg1 + arg2  # Calculate the result
    logging.info("[*] Result: {}".format(result))  # Log the result
    opcode = 2  # Set the opcode for the reply
    sock.send(int.to_bytes(OPCODE_REPLY, 4, "big"))  # Convert the opcode to bytes and send
    sock.send(int.to_bytes(result, 4, "big"))  # Convert the result to bytes and send

    sock.close()  # Close the socket connection

def run(addr, port):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((addr, port))

    server.listen(2)
    logging.info("[*] Server is Listening on {}:{}".format(addr, port))

    while True:
        client, info = server.accept()

        logging.info("[*] Server accept the connection from {}:{}".format(info[0], info[1]))

        client_handle = threading.Thread(target=protocol_execution, args=(client,))
        client_handle.start()

def command_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--addr", metavar="<server's IP address>", help="Server's IP address", type=str, default="0.0.0.0")
    parser.add_argument("-p", "--port", metavar="<server's open port>", help="Server's port", type=int, required=True)
    parser.add_argument("-l", "--log", metavar="<log level (DEBUG/INFO/WARNING/ERROR/CRITICAL)>", help="Log level (DEBUG/INFO/WARNING/ERROR/CRITICAL)", type=str, default="INFO")
    args = parser.parse_args()
    return args

def main():
    args = command_line_args()
    log_level = args.log
    logging.basicConfig(level=log_level)

    run(args.addr, args.port)

if __name__ == "__main__":
    main()
