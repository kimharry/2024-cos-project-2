#ifndef __BYTE_OP_H__ 
#define __BYTE_OP_H__  

#include <cstdio>  
#include <cstring>  

#define VAR_TO_MEM_1BYTE_BIG_ENDIAN(v, p) \
  *(p++) = v & 0xff; // Write the least significant byte of the value to memory and increment the pointer

#define VAR_TO_MEM_2BYTES_BIG_ENDIAN(v, p) \
  *(p++) = (v >> 8) & 0xff; *(p++) = v & 0xff; // Write the most significant byte of the value to memory, then the least significant byte, and increment the pointer twice

#define VAR_TO_MEM_4BYTES_BIG_ENDIAN(v, p) \
  *(p++) = (v >> 24) & 0xff; *(p++) = (v >> 16) & 0xff; *(p++) = (v >> 8) & 0xff; *(p++) = v & 0xff; // Write each byte of the value to memory, and increment the pointer four times

#define MEM_TO_VAR_1BYTE_BIG_ENDIAN(p, v) \
  v = (p[0] & 0xff); p += 1; // Read the byte from memory into the value and increment the pointer

#define MEM_TO_VAR_2BYTES_BIG_ENDIAN(p, v) \
  v = ((p[0] & 0xff) << 8) | (p[1] & 0xff); p += 2; // Read the two bytes from memory into the value and increment the pointer twice

#define MEM_TO_VAR_4BYTES_BIG_ENDIAN(p, v) \
  v = ((p[0] & 0xff) << 24) | ((p[1] & 0xff) << 16) | ((p[2] & 0xff) << 8) | (p[3] & 0xff); p += 4; // Read the four bytes from memory into the value and increment the pointer four times

#define PRINT_MEM(p, len) \
  printf("Print buffer:\n  >> "); \ 
  for (int i=0; i<len; i++) { \  
    printf("%02x ", p[i]); \  
    if (i % 16 == 15) printf("\n  >> "); \  
  } \  
  printf("\n"); //Print the memory buffer in hexadecimal format

// End of the byte operations header
#endif /* __BYTE_OP_H__ */ 

