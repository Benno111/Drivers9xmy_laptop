#ifndef IGD9X_WIN9X_DRIVER_H
#define IGD9X_WIN9X_DRIVER_H

#include "igd9x_win9x_exports.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#define IGD9X_WIN9X_ENTRY __stdcall
#else
#define IGD9X_WIN9X_ENTRY
#endif

IGD9X_WIN9X_EXPORT int IGD9X_WIN9X_ENTRY DriverEntry(void);

#ifdef __cplusplus
}
#endif

#endif

