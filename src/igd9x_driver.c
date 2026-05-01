#include <string.h>
#include "../include/igd9x_driver.h"

static igd9x_status_t igd9x_driver_probe_hw(const igd9x_driver_state_t *state)
{
    if (state == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    if (state->ops != 0 && state->ops->probe != 0) {
        return state->ops->probe(state->ops->context, &state->hw);
    }

    return igd9x_hw_probe(&state->hw);
}

igd9x_status_t igd9x_driver_attach(igd9x_driver_state_t *state,
                                   const igd9x_hw_info_t *hw,
                                   const igd9x_hw_ops_t *ops)
{
    const igd9x_mode_db_t *db;

    if (state == 0 || hw == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    memset(state, 0, sizeof(*state));
    state->hw = *hw;
    state->ops = ops;

    if (igd9x_driver_probe_hw(state) != IGD9X_STATUS_OK) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    db = igd9x_get_mode_db();
    state->native_mode = igd9x_find_mode(db->native_width,
                                         db->native_height,
                                         db->native_bpp,
                                         db->native_refresh_hz);
    if (state->native_mode == 0) {
        state->native_mode = igd9x_pick_best_mode(db->native_width,
                                                  db->native_height,
                                                  db->native_bpp,
                                                  db->native_refresh_hz);
    }
    state->current_mode = state->native_mode;
    if (state->current_mode != 0) {
        state->current_pitch = state->current_mode->pitch_bytes;
    }
    state->initialized = 1;

    return IGD9X_STATUS_OK;
}

void igd9x_driver_detach(igd9x_driver_state_t *state)
{
    if (state == 0) {
        return;
    }

    if (state->initialized && state->ops != 0 && state->ops->restore != 0) {
        state->ops->restore(state->ops->context);
    }

    memset(state, 0, sizeof(*state));
}

u16 igd9x_driver_mode_count(void)
{
    return igd9x_get_mode_db()->mode_count;
}

const igd9x_mode_t *igd9x_driver_mode_at(u16 index)
{
    return igd9x_get_mode_by_index(index);
}

igd9x_status_t igd9x_driver_enumerate_modes(const igd9x_driver_state_t *state,
                                            igd9x_mode_visit_fn visitor,
                                            void *context)
{
    const igd9x_mode_db_t *db;
    u16 i;

    if (state == 0 || visitor == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }
    if (!state->initialized) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    db = igd9x_get_mode_db();
    for (i = 0; i < db->mode_count; ++i) {
        visitor(&db->modes[i], context);
    }

    return IGD9X_STATUS_OK;
}

const igd9x_mode_t *igd9x_driver_get_native_mode(const igd9x_driver_state_t *state)
{
    if (state == 0) {
        return 0;
    }
    return state->native_mode;
}

const igd9x_mode_t *igd9x_driver_pick_mode(const igd9x_driver_state_t *state,
                                           u16 width,
                                           u16 height,
                                           u16 bpp,
                                           u16 refresh_hz)
{
    (void)state;
    return igd9x_pick_best_mode(width, height, bpp, refresh_hz);
}

igd9x_status_t igd9x_driver_set_mode(igd9x_driver_state_t *state,
                                     const igd9x_mode_t *mode)
{
    if (state == 0 || mode == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }
    if (!state->initialized) {
        return IGD9X_STATUS_UNSUPPORTED;
    }
    if (!igd9x_validate_mode(mode)) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    if (state->ops != 0 && state->ops->set_mode != 0) {
        if (state->ops->set_mode(state->ops->context, mode) != IGD9X_STATUS_OK) {
            return IGD9X_STATUS_HW_FAILURE;
        }
    } else if (igd9x_hw_set_mode(mode) != IGD9X_STATUS_OK) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    state->current_mode = mode;
    state->current_pitch = mode->pitch_bytes;

    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_driver_set_mode_by_request(igd9x_driver_state_t *state,
                                                u16 width,
                                                u16 height,
                                                u16 bpp,
                                                u16 refresh_hz)
{
    const igd9x_mode_t *mode;

    if (state == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    mode = igd9x_driver_pick_mode(state, width, height, bpp, refresh_hz);
    if (mode == 0) {
        return IGD9X_STATUS_NOT_FOUND;
    }

    return igd9x_driver_set_mode(state, mode);
}

const char *igd9x_driver_describe_mode(const igd9x_mode_t *mode, char *buffer, u16 buffer_len)
{
    return igd9x_mode_name(mode, buffer, buffer_len);
}

