#ifndef UTIL_H
#define UTIL_H

#include "incl.h"
#include "c_log.h"
#include "glad.h"

#include "mlib/m-dict.h"
DICT_DEF2(dict_uint, unsigned, unsigned)

void arr_i_print(const int *arr, const int len);
void arr_i_print2d(const int *arr, const int width, const int height);
void arr_f_print(double *arr, int len);
sds sdsfread(sds append_to, const char* path);

#endif
