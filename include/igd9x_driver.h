#ifndef IGD9X_DRIVER_H
#define IGD9X_DRIVER_H

#include "igd9x.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct igd9x_hw_ops {
    const char *name;
    void *context;
    igd9x_status_t (*probe)(void *context, const igd9x_hw_info_t *info);
    igd9x_status_t (*set_mode)(void *context, const igd9x_mode_t *mode);
    igd9x_status_t (*restore)(void *context);
} igd9x_hw_ops_t;

typedef struct igd9x_driver_state {
    igd9x_hw_info_t hw;
    const igd9x_hw_ops_t *ops;
    const igd9x_mode_t *native_mode;
    const igd9x_mode_t *current_mode;
    u16 current_pitch;
    u8 initialized;
} igd9x_driver_state_t;

typedef void (*igd9x_mode_visit_fn)(const igd9x_mode_t *mode, void *context);

igd9x_status_t igd9x_driver_attach(igd9x_driver_state_t *state,
                                   const igd9x_hw_info_t *hw,
                                   const igd9x_hw_ops_t *ops);
void igd9x_driver_detach(igd9x_driver_state_t *state);
u16 igd9x_driver_mode_count(void);
const igd9x_mode_t *igd9x_driver_mode_at(u16 index);
igd9x_status_t igd9x_driver_enumerate_modes(const igd9x_driver_state_t *state,
                                            igd9x_mode_visit_fn visitor,
                                            void *context);
const igd9x_mode_t *igd9x_driver_get_native_mode(const igd9x_driver_state_t *state);
const igd9x_mode_t *igd9x_driver_pick_mode(const igd9x_driver_state_t *state,
                                           u16 width,
                                           u16 height,
                                           u16 bpp,
                                           u16 refresh_hz);
igd9x_status_t igd9x_driver_set_mode(igd9x_driver_state_t *state,
                                     const igd9x_mode_t *mode);
igd9x_status_t igd9x_driver_set_mode_by_request(igd9x_driver_state_t *state,
                                                u16 width,
                                                u16 height,
                                                u16 bpp,
                                                u16 refresh_hz);
const char *igd9x_driver_describe_mode(const igd9x_mode_t *mode, char *buffer, u16 buffer_len);

extern const igd9x_hw_ops_t igd9x_hw_stub_ops;

#ifdef __cplusplus
}
#endif

#endif

