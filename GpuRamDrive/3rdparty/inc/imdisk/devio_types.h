
#if defined(_MSC_VER) && _MSC_VER < 1900

#define PRIu64 "I64u"
#define PRIi64 "I64i"

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;

#else

#include <inttypes.h>

#endif

#define ULL_FMT         "%" PRIu64
#define SLL_FMT         "%" PRIi64

#ifdef _WIN32

typedef ULONG safeio_size_t;
typedef LONG safeio_ssize_t;
typedef LONGLONG off_t_64;
typedef int socklen_t;

#define SIZ_FMT         "%u"
#define SSZ_FMT         "%i"

#else

typedef size_t safeio_size_t;
typedef ssize_t safeio_ssize_t;
typedef off_t off_t_64;
typedef int SOCKET;

#define SIZ_FMT         "%zu"
#define SSZ_FMT         "%zi"

#define INVALID_SOCKET (-1)

#define _lseeki64       lseek
#define closesocket     close
#define _flushall       flushall
#define _open           open
#define _close          close
#define _stricmp        strcasecmp
#define _strnicmp       strncasecmp

#ifndef O_BINARY
#define O_BINARY       0
#endif

#ifndef __cdecl
#define __cdecl
#endif

#endif
