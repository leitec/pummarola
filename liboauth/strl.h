#if defined(__OpenBSD__) || defined(__APPLE__)
#define HAVE_STRL_FUNCTIONS 1
#endif

#ifndef HAVE_STRL_FUNCTIONS
#define NO_STRL
#define strlcat __openbsd_strlcat
#define strlcpy __openbsd_strlcpy

size_t __openbsd_strlcat(char *dst, const char *src, size_t siz);
size_t __openbsd_strlcpy(char *dst, const char *src, size_t siz);
#endif
