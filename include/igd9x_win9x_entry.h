#ifndef IGD9X_WIN9X_ENTRY_H
#define IGD9X_WIN9X_ENTRY_H

#include "igd9x_intel.h"
#include "igd9x_win9x_mmio.h"
#include "igd9x_win9x_pci.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct igd9x_win9x_entry_state {
    igd9x_driver_state_t driver;
    igd9x_pci_function_t pci;
    igd9x_hw_info_t hw;
    u8 pci_discovered;
    u8 mmio_bound;
    u8 attached;
} igd9x_win9x_entry_state_t;

igd9x_status_t igd9x_win9x_entry_init(igd9x_win9x_entry_state_t *state);
void igd9x_win9x_entry_shutdown(igd9x_win9x_entry_state_t *state);
igd9x_status_t igd9x_win9x_entry_discover(igd9x_win9x_entry_state_t *state);
igd9x_status_t igd9x_win9x_entry_attach(igd9x_win9x_entry_state_t *state);
igd9x_status_t igd9x_win9x_entry_set_native_mode(igd9x_win9x_entry_state_t *state);
igd9x_status_t igd9x_win9x_entry_set_mode_request(igd9x_win9x_entry_state_t *state,
                                                  u16 width,
                                                  u16 height,
                                                  u16 bpp,
                                                  u16 refresh_hz);
igd9x_status_t igd9x_win9x_entry_enumerate_modes(const igd9x_win9x_entry_state_t *state,
                                                 igd9x_mode_visit_fn visitor,
                                                 void *context);
const igd9x_pci_function_t *igd9x_win9x_entry_get_pci(const igd9x_win9x_entry_state_t *state);
const igd9x_hw_info_t *igd9x_win9x_entry_get_hw(const igd9x_win9x_entry_state_t *state);
const igd9x_mode_t *igd9x_win9x_entry_get_current_mode(const igd9x_win9x_entry_state_t *state);

#ifdef __cplusplus
}
#endif

#endif

