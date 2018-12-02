#pragma once
#include <stdio.h>

#ifdef DEBUG
#define dbg_printf printf
#else
#define dbg_printf(...)
#endif
