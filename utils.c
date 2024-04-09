/*
 * relaxisloader
 * Copyright (C) Carl Philipp Klemm 2023 <carl@uvos.xyz>
 *
 * relaxisloader is free software: you can redistribute it and/or modify it
 * under the terms of the lesser GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * relaxisloader is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the lesser GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "strptime.h"

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
