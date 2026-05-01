#ifndef IGD9X_WIN9X_EXPORTS_H
#define IGD9X_WIN9X_EXPORTS_H

#include "igd9x_win9x_entry.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#define IGD9X_WIN9X_EXPORT __declspec(dllexport)
#else
#define IGD9X_WIN9X_EXPORT
#endif

IGD9X_WIN9X_EXPORT igd9x_status_t igd9x_win9x_driver_startup(void);
IGD9X_WIN9X_EXPORT void igd9x_win9x_driver_shutdown(void);
IGD9X_WIN9X_EXPORT igd9x_status_t igd9x_win9x_driver_discover(void);
IGD9X_WIN9X_EXPORT igd9x_status_t igd9x_win9x_driver_attach(void);
IGD9X_WIN9X_EXPORT igd9x_status_t igd9x_win9x_driver_set_native_mode(void);
IGD9X_WIN9X_EXPORT igd9x_status_t igd9x_win9x_driver_set_mode_request(u16 width,
                                                                      u16 height,
                                                                      u16 bpp,
                                                                      u16 refresh_hz);
IGD9X_WIN9X_EXPORT igd9x_status_t igd9x_win9x_driver_enumerate_modes(igd9x_mode_visit_fn visitor,
                                                                     void *context);
IGD9X_WIN9X_EXPORT const igd9x_win9x_entry_state_t *igd9x_win9x_driver_state(void);

#ifdef __cplusplus
}
#endif

#endif

