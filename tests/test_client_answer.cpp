#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <getopt.h>
#include <stdint.h>

#include "../edge/byte_op.h"

#define BUFLEN        1024
#define OPCODE_SUM    1
#define OPCODE_REPLY  2

void protocol_execution(int sock);
void error_handling(const char *message);

void usage(const char *pname)
{
  printf(">> Usage: %s [options]\n", pname);
  printf("Options\n");
  printf("  -a, --addr       Server's address\n");
  printf("  -p, --port       Server's port\n");
  exit(0);
}

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
  char msg[] = "Hello, World!\n";
	char message[30] = {0, };
	int c, port, tmp, str_len;
  char *pname;
  uint8_t *addr;
  uint8_t eflag;

  pname = argv[0];
  addr = NULL;
  port = -1;
  eflag = 0;

  while (1)
  {
    int option_index = 0;
    static struct option long_options[] = {
      {"addr", required_argument, 0, 'a'},
      {"port", required_argument, 0, 'p'},
      {0, 0, 0, 0}
    };

    const char *opt = "a:p:0";

    c = getopt_long(argc, argv, opt, long_options, &option_index);

    if (c == -1)
      break;

    switch (c)
    {
      case 'a':
        tmp = strlen(optarg);
        addr = (uint8_t *)malloc(tmp);
        memcpy(addr, optarg, tmp);
        break;

      case 'p':
        port = atoi(optarg);
        break;

      default:
        usage(pname);
    }
  }

  if (!addr)
  {
    printf("[*] Please specify the server's address to connect\n");
    eflag = 1;
  }

  if (port < 0)
  {
    printf("[*] Please specify the server's port to connect\n");
    eflag = 1;
  }

  if (eflag)
  {
    usage(pname);
    exit(0);
  }

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		error_handling("socket() error");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr((const char *)addr);
	serv_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
  printf("[*] Connected to %s:%d\n", addr, port);
  
  protocol_execution(sock);

	close(sock);
	return 0;
}

void protocol_execution(int sock)
{
  char msg[] = "Alice"; // The name to be sent
  char buf[BUFLEN]; // Buffer for receiving data
  int tbs, sent, tbr, rcvd, offset; // Variables for tracking the number of bytes and the offset
  int len; // Length of the name

  // tbs: the number of bytes to send
  // tbr: the number of bytes to receive
  // offset: the offset of the message

  // 1. Alice -> Bob: length of the name (4 bytes) || name (length bytes)
  // Send the length information (4 bytes)
  len = strlen(msg);
  printf("[*] Length information to be sent: %d\n", len);

  len = htonl(len); // Convert the length to network byte order
  tbs = 4; // Number of bytes to send 
  offset = 0; // Offset for tracking the progress of sending

  while (offset < tbs)
  {
    sent = write(sock, &len + offset, tbs - offset); // Send the length information
    if (sent > 0)
      offset += sent; // Update the offset
  }

  // Send the name (Alice)
  tbs = ntohl(len); // Convert the length back to host byte order
  offset = 0; // Reset the offset

  printf("[*] Name to be sent: %s\n", msg);  // Print the name to be sent
  while (offset < tbs)  // While there are bytes left to send
  {
    sent = write(sock, msg + offset, tbs - offset); // Send the name
    if (sent > 0)
      offset += sent; // Update the offset
  }

  // 2. Bob -> Alice: length of the name (4 bytes) || name (length bytes)
  // Receive the length information (4 bytes)
  tbr = 4; // Number of bytes to receive 
  offset = 0; // Reset the offset

  while (offset < tbr)  // While there are bytes left to receive
  {
    rcvd = read(sock, &len + offset, tbr - offset); // Receive the length information
    if (rcvd > 0)
      offset += rcvd; // Update the offset
  }
  len = ntohl(len); // Convert the length back to host byte order
  printf("[*] Length received: %d\n", len);

  // Receive the name (Bob)
  tbr = len; // Number of bytes to receive (name)
  offset = 0; // Reset the offset

  while (offset < tbr)  // While there are bytes left to receive
  {
    rcvd = read(sock, buf + offset, tbr - offset); // Receive the name
    if (rcvd > 0)
      offset += rcvd; // Update the offset
  }

  printf("[*] Name received: %s \n", buf);  // Print the received name

  // Implement following the instructions below
  // Let's assume there are two opcodes:
  //     1: summation request for the two arguments
  //     2: reply with the result
  // 3. Alice -> Bob: opcode (4 bytes) || arg1 (4 bytes) || arg2 (4 bytes)
  // The opcode should be 1

  char *p;  // Pointer for handling the buffer
  int i, arg1, arg2;  // Variables for handling the arguments

  memset(buf, 0, BUFLEN); // Clear the buffer
  p = buf; // Pointer to the buffer
  arg1 = 2; // First argument
  arg2 = 5; // Second argument

  VAR_TO_MEM_1BYTE_BIG_ENDIAN(OPCODE_SUM, p); // Convert the opcode to network byte order & store it in the buffer
  VAR_TO_MEM_4BYTES_BIG_ENDIAN(arg1, p); // Convert the first argument to network byte order & store it in the buffer
  VAR_TO_MEM_4BYTES_BIG_ENDIAN(arg2, p); // Convert the second argument to network byte order & store it in the buffer
  tbs = p - buf; // Calculate the number of bytes to send
  offset = 0; // Reset the offset

  printf("[*] # of bytes to be sent: %d\n", tbs);  // Print the number of bytes to be sent
  printf("[*] The following bytes will be sent\n");  // Print the bytes to be sent
  for (i=0; i<tbs; i++)  // For each byte to be sent
    printf ("%02x ", buf[i]);  // Print the byte
  printf("\n");  // Print a newline

  while (offset < tbs)  // While there are bytes left to send
  {
    sent = write(sock, buf + offset, tbs - offset); // Send the data
    if (sent > 0)
      offset += sent; // Update the offset
  }

  // 4. Bob -> Alice: opcode (4 bytes) || result (4 bytes)
  // The opcode should be 2

  int opcode, result;  // Variables for storing the opcode & the result

  tbr = 8; // Number of bytes to receive (opcode + result)
  offset = 0; // Reset the offset
  memset(buf, 0, BUFLEN); // Clear the buffer

  printf("[*] # of bytes to be received: %d\n", tbr);  // Print the number of bytes to be received
  while (offset < tbr)  // While there are bytes left to receive
  {
    rcvd = read(sock, buf + offset, tbs - offset); // Receive the data
    if (rcvd > 0)  // If bytes were received
      offset += rcvd; // Update the offset
  }

  printf("[*] The following bytes is received\n");  // Print the received bytes
  for (i=0; i<tbr; i++)  // For each received byte
    printf("%02x ", buf[i]);  // Print the byte
  printf("\n");

  p = buf; // Reset the pointer to the buffer
  MEM_TO_VAR_4BYTES_BIG_ENDIAN(p, opcode); // Convert the opcode from network byte order and store it in the variable
  printf("[*] Opcode: %d\n", opcode);
  MEM_TO_VAR_4BYTES_BIG_ENDIAN(p, result); // Convert the result from network byte order and store it in the variable
  printf("[*] Result: %d\n", result);
}

void error_handling(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
