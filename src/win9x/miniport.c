#include "../../include/igd9x_driver.h"

igd9x_status_t igd9x_win9x_miniport_attach(igd9x_driver_state_t *state,
                                           const igd9x_hw_info_t *hw,
                                           const igd9x_hw_ops_t *ops)
{
    return igd9x_driver_attach(state, hw, ops);
}

void igd9x_win9x_miniport_detach(igd9x_driver_state_t *state)
{
    igd9x_driver_detach(state);
}

igd9x_status_t igd9x_win9x_miniport_set_mode(igd9x_driver_state_t *state,
                                             const igd9x_mode_t *mode)
{
    return igd9x_driver_set_mode(state, mode);
}

igd9x_status_t igd9x_win9x_miniport_set_mode_by_request(igd9x_driver_state_t *state,
                                                        u16 width,
                                                        u16 height,
                                                        u16 bpp,
                                                        u16 refresh_hz)
{
    return igd9x_driver_set_mode_by_request(state, width, height, bpp, refresh_hz);
}

igd9x_status_t igd9x_win9x_miniport_enumerate_modes(const igd9x_driver_state_t *state,
                                                    igd9x_mode_visit_fn visitor,
                                                    void *context)
{
    return igd9x_driver_enumerate_modes(state, visitor, context);
}

