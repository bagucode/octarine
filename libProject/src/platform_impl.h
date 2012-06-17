
#ifndef octarine_platform_impl_h
#define octarine_platform_impl_h

#ifdef __APPLE__
#include "platform_impl_MacOSX.h"
#elif WIN32
#include "platform_impl_MSWIN.h"
#else
#include "platform_impl_Linux.h"
#endif


#endif
