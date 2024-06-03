#include "network_manager.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>

#include "opcode.h"
using namespace std;

NetworkManager::NetworkManager() 
{
  this->sock = -1;
  this->addr = NULL;
  this->port = -1;
}

NetworkManager::NetworkManager(const char *addr, int port)
{
  this->sock = -1;
  this->addr = addr;
  this->port = port;
}

void NetworkManager::setAddress(const char *addr)
{
  this->addr = addr;
}

const char *NetworkManager::getAddress()
{
  return this->addr;
}

void NetworkManager::setPort(int port)
{
  this->port = port;
}

int NetworkManager::getPort()
{
  return this->port;
}

int NetworkManager::init()
{
	struct sockaddr_in serv_addr;

	this->sock = socket(PF_INET, SOCK_STREAM, 0);
	if (this->sock == FAILURE)
  {
    cout << "[*] Error: socket() error" << endl;
    cout << "[*] Please try again" << endl;
    exit(1);
  }

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(this->addr);
	serv_addr.sin_port = htons(this->port);

	if (connect(this->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == FAILURE)
  {
    cout << "[*] Error: connect() error" << endl;
    cout << "[*] Please try again" << endl;
    exit(1);
  }
	
  cout << "[*] Connected to " << this->addr << ":" << this->port << endl;

  return sock;
}

// TODO: You should revise the following code
int NetworkManager::sendData(uint8_t *data, int dlen)
{
  int sock, tbs, sent, offset, num, jlen;  // Declare variables for the socket, total bytes to send, bytes sent, offset, number of houses, and JSON length
  unsigned char opcode;  // Declare a variable for the opcode
  uint8_t n[4];  // Declare a 4-byte array
  uint8_t *p;  // Declare a pointer to a byte

  sock = this->sock;  // Get the socket from the current object

  // Example) data (processed by ProcessManager) consists of:
  // Example) minimum temperature (1 byte) || minimum humidity (1 byte) || minimum power (2 bytes) || month (1 byte)
  // Example) edge -> server: opcode (OPCODE_DATA, 1 byte)
  opcode = OPCODE_DATA;  // Set the opcode to OPCODE_DATA
  tbs = 1; offset = 0;  // Initialize the total bytes to send and the offset
  while (offset < tbs)  // While the offset is less than the total bytes to send
  {
    sent = write(sock, &opcode + offset, tbs - offset);  // Write the opcode to the socket
    if (sent > 0)  // If the write was successful
      offset += sent;  // Increase the offset by the number of bytes sent
  }
  assert(offset == tbs);  // Assert that all bytes were sent

  // Example) edge -> server: temperature (1 byte) || humidity (1 byte) || power (2 bytes) || month (1 byte)
  tbs = 5; offset = 0;  // Initialize the total bytes to send and the offset
  while (offset < tbs)  // While the offset is less than the total bytes to send
  {
    sent = write(sock, data + offset, tbs - offset);  // Write the data to the socket
    if (sent > 0)  // If the write was successful
      offset += sent;  // Increase the offset by the number of bytes sent
  }
  assert(offset == tbs);  // Assert that all bytes were sent

  return 0;  // Return 0 to indicate success
}

// TODO: Please revise or implement this function as you want. You can also remove this function if it is not needed
uint8_t NetworkManager::receiveCommand() 
{
  int sock;  // Declare a variable for the socket
  uint8_t opcode;  // Declare a variable for the opcode
  uint8_t *p;  // Declare a pointer to a byte

  sock = this->sock;  // Get the socket from the current object
  opcode = OPCODE_WAIT;  // Initialize the opcode to OPCODE_WAIT

  while (opcode == OPCODE_WAIT)  // While the opcode is OPCODE_WAIT
    read(sock, &opcode, 1);  // Read one byte from the socket

  assert(opcode == OPCODE_DONE || opcode == OPCODE_QUIT);  // Assert that the opcode is either OPCODE_DONE or OPCODE_QUIT

  return opcode;  // Return the opcode
}
