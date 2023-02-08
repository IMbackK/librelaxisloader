#pragma once
#include <time.h>

char *rlx_strconcat(const char* a, const char* b);
char *rlx_strdup(const char* a);
time_t rlx_str_to_time(const char* str);
char *rlx_alloc_printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
