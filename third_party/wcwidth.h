#ifndef WCWIDTH_H_INCLUDED
#define WCWIDTH_H_INCLUDED

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

int termux_wcwidth(wchar_t ucs);

#ifdef __cplusplus
}
#endif

#endif
