#pragma once
#include <stdio.h>
//#define DEBUG_US

#ifdef DEBUG_US
#define US_DEBUG(...) printf(__VA_ARGS__)
#else
#define US_DEBUG(...)
#endif