#include "../include/igd9x.h"
#include "../include/igd9x_driver.h"

#define IGD9X_INTEL_VENDOR_ID 0x8086U

typedef struct igd9x_hw_stub_state {
    const igd9x_mode_t *last_mode;
    u16 last_pitch;
    u32 last_clock_khz;
} igd9x_hw_stub_state_t;

static igd9x_hw_stub_state_t g_stub_state;

igd9x_status_t igd9x_hw_probe(const igd9x_hw_info_t *info)
{
    if (info == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    if (info->vendor_id != IGD9X_INTEL_VENDOR_ID) {
        return IGD9X_STATUS_NOT_FOUND;
    }

    if (((info->class_code >> 24) & 0xFFUL) != 0x03UL) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_hw_set_mode(const igd9x_mode_t *mode)
{
    if (mode == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    return IGD9X_STATUS_OK;
}

static igd9x_status_t igd9x_stub_probe(void *context, const igd9x_hw_info_t *info)
{
    (void)context;
    return igd9x_hw_probe(info);
}

static igd9x_status_t igd9x_stub_set_mode(void *context, const igd9x_mode_t *mode)
{
    igd9x_hw_stub_state_t *state;

    state = (igd9x_hw_stub_state_t *)context;
    if (mode == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }
    if (!igd9x_validate_mode(mode)) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    if (state != 0) {
        state->last_mode = mode;
        state->last_pitch = mode->pitch_bytes;
        state->last_clock_khz = mode->pixel_clock_khz;
    }

    return igd9x_hw_set_mode(mode);
}

static igd9x_status_t igd9x_stub_restore(void *context)
{
    igd9x_hw_stub_state_t *state;

    state = (igd9x_hw_stub_state_t *)context;
    if (state != 0) {
        state->last_mode = 0;
        state->last_pitch = 0;
        state->last_clock_khz = 0;
    }
    return IGD9X_STATUS_OK;
}

const igd9x_hw_ops_t igd9x_hw_stub_ops = {
    "intel-hd-4000-stub",
    &g_stub_state,
    igd9x_stub_probe,
    igd9x_stub_set_mode,
    igd9x_stub_restore
};
