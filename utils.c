#include "utils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

char *rlx_strconcat(const char* a, const char* b)
{
	char* full_str = malloc(strlen(a)+strlen(b));
	assert(full_str);
	strcpy(full_str, a);
	strcat(full_str, b);
	return full_str;
}

char *rlx_strdup(const char* a)
{
	char *ret = malloc(strlen(a)+1);
	assert(ret);
	strcpy(ret, a);
	return ret;
}

time_t rlx_str_to_time(const char* str)
{
	struct tm tm = {};
	char *tmRet = strptime(str, "%Y-%m-%d%t%H:%M:%S", &tm);
	assert(tmRet);
	return mktime(&tm);
}

char *rlx_alloc_printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	char *out = malloc(len+1);
	va_start(args, fmt);
	vsnprintf(out, len+1, fmt, args);
	va_end(args);
	return out;
}
