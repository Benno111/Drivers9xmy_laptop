#include <string.h>
#include "../../include/igd9x_win9x_exports.h"

static igd9x_win9x_entry_state_t g_win9x_driver_state;
static u8 g_win9x_driver_started;

igd9x_status_t igd9x_win9x_driver_startup(void)
{
    igd9x_status_t status;

    if (g_win9x_driver_started) {
        return IGD9X_STATUS_OK;
    }

    status = igd9x_win9x_entry_init(&g_win9x_driver_state);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    g_win9x_driver_started = 1;
    return IGD9X_STATUS_OK;
}

void igd9x_win9x_driver_shutdown(void)
{
    if (!g_win9x_driver_started) {
        return;
    }

    igd9x_win9x_entry_shutdown(&g_win9x_driver_state);
    memset(&g_win9x_driver_state, 0, sizeof(g_win9x_driver_state));
    g_win9x_driver_started = 0;
}

igd9x_status_t igd9x_win9x_driver_discover(void)
{
    if (!g_win9x_driver_started) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    return igd9x_win9x_entry_discover(&g_win9x_driver_state);
}

igd9x_status_t igd9x_win9x_driver_attach(void)
{
    if (!g_win9x_driver_started) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    return igd9x_win9x_entry_attach(&g_win9x_driver_state);
}

igd9x_status_t igd9x_win9x_driver_set_native_mode(void)
{
    if (!g_win9x_driver_started) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    return igd9x_win9x_entry_set_native_mode(&g_win9x_driver_state);
}

igd9x_status_t igd9x_win9x_driver_set_mode_request(u16 width,
                                                   u16 height,
                                                   u16 bpp,
                                                   u16 refresh_hz)
{
    if (!g_win9x_driver_started) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    return igd9x_win9x_entry_set_mode_request(&g_win9x_driver_state,
                                              width,
                                              height,
                                              bpp,
                                              refresh_hz);
}

igd9x_status_t igd9x_win9x_driver_enumerate_modes(igd9x_mode_visit_fn visitor,
                                                  void *context)
{
    if (!g_win9x_driver_started) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    return igd9x_win9x_entry_enumerate_modes(&g_win9x_driver_state, visitor, context);
}

const igd9x_win9x_entry_state_t *igd9x_win9x_driver_state(void)
{
    if (!g_win9x_driver_started) {
        return 0;
    }

    return &g_win9x_driver_state;
}

