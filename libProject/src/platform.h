#ifndef octarine_platform_h
#define octarine_platform_h

#ifdef __APPLE__
#include "platform_MacOSX.h"
#elif WIN32
#include "platform_MSWIN.h"
#else
#include "platform_Linux.h"
#endif
