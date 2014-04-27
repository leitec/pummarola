#ifndef __LIBEITEC_H
#define __LIBEITEC_H

#ifdef PLAN9
#define _POSIX_SOURCE
#define _BSD_EXTENSION
#define _POSIX_EXTENSION
#include <errno.h>

#define uint32_t unsigned long
#define uint16_t unsigned short
#define uint8_t unsigned char
#define int32_t long
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef __MINGW32__
#include <stdint.h>
#else
#ifndef macintosh
#include <sys/types.h>
#endif
#endif

#ifdef _MSC_VER
#define __MINGW32__
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
#else
#include <unistd.h>
#endif

#if defined(__APPLE__) || defined(linux)
#include <stdint.h>
#endif

#ifdef __DJGPP__
#ifndef __dj_stdint__h_
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#endif
#endif

#ifdef macintosh
#include "libcompat.h"
#endif

#include "lc_byte.h"
#include "lc_list.h"
#include "lc_md5.h"

#include "lc_protos.h"
#endif
