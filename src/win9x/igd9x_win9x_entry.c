#include <string.h>
#include "../../include/igd9x_win9x_entry.h"

static igd9x_status_t igd9x_win9x_entry_bind_mmio(igd9x_win9x_entry_state_t *state)
{
    igd9x_status_t status;

    if (state == 0 || !state->pci_discovered) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    status = igd9x_win9x_mmio_bind(state->pci.bar0, state->pci.bar0_size);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    status = igd9x_intel_backend_bind_mmio(igd9x_intel_backend_ops.context,
                                           &igd9x_win9x_mmio_ops);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    state->mmio_bound = 1;
    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_win9x_entry_init(igd9x_win9x_entry_state_t *state)
{
    if (state == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    memset(state, 0, sizeof(*state));
    return IGD9X_STATUS_OK;
}

void igd9x_win9x_entry_shutdown(igd9x_win9x_entry_state_t *state)
{
    if (state == 0) {
        return;
    }

    if (state->attached) {
        igd9x_driver_detach(&state->driver);
    }

    memset(state, 0, sizeof(*state));
}

igd9x_status_t igd9x_win9x_entry_discover(igd9x_win9x_entry_state_t *state)
{
    igd9x_status_t status;

    if (state == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    status = igd9x_pci_discover_intel_display(&igd9x_win9x_pci_ops, &state->pci);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    status = igd9x_pci_function_to_hw_info(&state->pci, &state->hw);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    status = igd9x_intel_backend_import_pci(igd9x_intel_backend_ops.context, &state->pci);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    state->pci_discovered = 1;
    return igd9x_win9x_entry_bind_mmio(state);
}

igd9x_status_t igd9x_win9x_entry_attach(igd9x_win9x_entry_state_t *state)
{
    igd9x_status_t status;

    if (state == 0 || !state->pci_discovered || !state->mmio_bound) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    status = igd9x_driver_attach(&state->driver, &state->hw, &igd9x_intel_backend_ops);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    state->attached = 1;
    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_win9x_entry_set_native_mode(igd9x_win9x_entry_state_t *state)
{
    const igd9x_mode_db_t *db;

    if (state == 0 || !state->attached) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    db = igd9x_get_mode_db();
    return igd9x_driver_set_mode_by_request(&state->driver,
                                            db->native_width,
                                            db->native_height,
                                            db->native_bpp,
                                            db->native_refresh_hz);
}

igd9x_status_t igd9x_win9x_entry_set_mode_request(igd9x_win9x_entry_state_t *state,
                                                  u16 width,
                                                  u16 height,
                                                  u16 bpp,
                                                  u16 refresh_hz)
{
    if (state == 0 || !state->attached) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    return igd9x_driver_set_mode_by_request(&state->driver,
                                            width,
                                            height,
                                            bpp,
                                            refresh_hz);
}

igd9x_status_t igd9x_win9x_entry_enumerate_modes(const igd9x_win9x_entry_state_t *state,
                                                 igd9x_mode_visit_fn visitor,
                                                 void *context)
{
    if (state == 0 || !state->attached) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    return igd9x_driver_enumerate_modes(&state->driver, visitor, context);
}

const igd9x_pci_function_t *igd9x_win9x_entry_get_pci(const igd9x_win9x_entry_state_t *state)
{
    if (state == 0 || !state->pci_discovered) {
        return 0;
    }

    return &state->pci;
}

const igd9x_hw_info_t *igd9x_win9x_entry_get_hw(const igd9x_win9x_entry_state_t *state)
{
    if (state == 0 || !state->pci_discovered) {
        return 0;
    }

    return &state->hw;
}

const igd9x_mode_t *igd9x_win9x_entry_get_current_mode(const igd9x_win9x_entry_state_t *state)
{
    if (state == 0 || !state->attached) {
        return 0;
    }

    return state->driver.current_mode;
}

