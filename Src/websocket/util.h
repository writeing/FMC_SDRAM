#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _SSIZE_T_DEFINED
typedef int ssize_t;
#define _SSIZE_T_DEFINED
#endif
//#ifndef _SOCKET_T_DEFINED
//typedef SOCKET socket_t;
//#define _SOCKET_T_DEFINED
//#endif
//#ifndef snprintf
//#define snprintf _snprintf
//#define close closesocket
#include <stdint.h>
//#else
//typedef __int8 int8_t;
//typedef unsigned __int8 uint8_t;
//typedef __int32 int32_t;
//typedef unsigned __int32 uint32_t;
//typedef unsigned __int16 uint16_t;
//typedef __int64 int64_t;
//typedef unsigned __int64 uint64_t;
//#endif
//#define socketerrno WSAGetLastError()
//#define SOCKET_EAGAIN_EINPROGRESS WSAEINPROGRESS
//#define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK


//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <string.h>
//#ifndef INVALID_SOCKET
//#define INVALID_SOCKET (-1)
//#endif
//#ifndef SOCKET_ERROR
//#define SOCKET_ERROR (-1)
//#endif
//#include <errno.h>
//#define socketerrno errno
//#define SOCKET_EAGAIN_EINPROGRESS EAGAIN
//#define SOCKET_EWOULDBLOCK EWOULDBLOCK
//#endif
//#ifndef _WIN32
//uint64_t ntohll(uint64_t val);
//uint64_t htonll(uint64_t val);
//#endif
typedef struct
{
    uint32_t state[5];
    uint32_t count[2];
    uint8_t  buffer[64];
} SHA1_CTX;

#define SHA1_DIGEST_SIZE 20

int32_t ut_connect(const char *hostname, uint16_t port);
uint8_t *sha1Buff(const void *buff,size_t lenth, uint8_t *out);
uint8_t *base64_encode(uint8_t *bindata, int32_t inlen, uint8_t *out, int32_t *outlen);
char *str2lower(char *str);

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

