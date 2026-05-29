#include <string.h>
#include "../../include/igd9x_intel.h"

#define IGD9X_INTEL_VENDOR_ID 0x8086U
#define IGD9X_INTEL_CLASS_DISPLAY 0x03000000UL

static igd9x_intel_backend_state_t g_intel_backend_state;

static u32 igd9x_intel_pack_htotal(const igd9x_mode_t *mode)
{
    return ((u32)(mode->htotal - 1U) << 16) | (u32)(mode->width - 1U);
}

static u32 igd9x_intel_pack_hblank(const igd9x_mode_t *mode)
{
    return ((u32)(mode->htotal - 1U) << 16) | (u32)(mode->width - 1U);
}

static u32 igd9x_intel_pack_hsync(const igd9x_mode_t *mode)
{
    return ((u32)(mode->hsync_end - 1U) << 16) | (u32)(mode->hsync_start - 1U);
}

static u32 igd9x_intel_pack_vtotal(const igd9x_mode_t *mode)
{
    return ((u32)(mode->vtotal - 1U) << 16) | (u32)(mode->height - 1U);
}

static u32 igd9x_intel_pack_vblank(const igd9x_mode_t *mode)
{
    return ((u32)(mode->vtotal - 1U) << 16) | (u32)(mode->height - 1U);
}

static u32 igd9x_intel_pack_vsync(const igd9x_mode_t *mode)
{
    return ((u32)(mode->vsync_end - 1U) << 16) | (u32)(mode->vsync_start - 1U);
}

static u32 igd9x_intel_pack_src(const igd9x_mode_t *mode)
{
    return ((u32)(mode->height - 1U) << 16) | (u32)(mode->width - 1U);
}

static u32 igd9x_intel_plane_control(const igd9x_mode_t *mode, u8 pipe)
{
    u32 value;
    u32 format;

    value = IGD9X_INTEL_DSPCNTR_ENABLE | IGD9X_INTEL_DSPCNTR_GAMMA;
    value |= (pipe == 0) ? IGD9X_INTEL_DSPCNTR_SEL_PIPE_A : IGD9X_INTEL_DSPCNTR_SEL_PIPE_B;
    format = 0;
    switch (mode->bpp) {
    case 8:
    case 16:
        format = IGD9X_INTEL_DSPCNTR_FMT_16BPP;
        break;
    case 24:
    case 32:
        format = IGD9X_INTEL_DSPCNTR_FMT_32BPP;
        break;
    default:
        break;
    }

    if (format == 0) {
        return 0UL;
    }

    value |= format;
    return value;
}

static u32 igd9x_intel_pipe_control(void)
{
    return IGD9X_INTEL_PIPECONF_ENABLE | IGD9X_INTEL_PIPECONF_GAMMA;
}

static igd9x_status_t igd9x_intel_wait_for_bit(const igd9x_mmio_ops_t *mmio,
                                               u32 reg,
                                               u32 mask,
                                               u32 expect_set)
{
    u32 i;
    u32 value;

    if (mmio == 0 || mmio->read32 == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0; i < 100000U; ++i) {
        value = mmio->read32(mmio->context, reg);
        if (expect_set) {
            if ((value & mask) == mask) {
                return IGD9X_STATUS_OK;
            }
        } else if ((value & mask) == 0) {
            return IGD9X_STATUS_OK;
        }
    }

    return IGD9X_STATUS_HW_FAILURE;
}

static igd9x_status_t igd9x_intel_enable_internal_panel(const igd9x_mmio_ops_t *mmio)
{
    igd9x_status_t status;

    if (mmio == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    status = igd9x_mmio_write32(mmio, IGD9X_INTEL_MMIO_PP_ON_DELAYS, 0x01900310UL);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }
    status = igd9x_mmio_write32(mmio, IGD9X_INTEL_MMIO_PP_OFF_DELAYS, 0x00190031UL);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }
    status = igd9x_mmio_write32(mmio, IGD9X_INTEL_MMIO_PP_DIVISOR, 0x00000001UL);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    status = igd9x_mmio_write32(mmio, IGD9X_INTEL_MMIO_PP_CONTROL, IGD9X_INTEL_PANEL_POWER_ENABLE);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    status = igd9x_intel_wait_for_bit(mmio, IGD9X_INTEL_MMIO_PP_STATUS,
                                      IGD9X_INTEL_PANEL_POWER_ENABLE, 1);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    status = igd9x_mmio_write32(mmio, IGD9X_INTEL_MMIO_LVDS_CTL, IGD9X_INTEL_LVDS_ENABLE);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    return IGD9X_STATUS_OK;
}

static void igd9x_intel_plan_add(igd9x_intel_timing_plan_t *plan, u32 reg, u32 value)
{
    if (plan->write_count < (u16)(sizeof(plan->writes) / sizeof(plan->writes[0]))) {
        plan->writes[plan->write_count].reg = reg;
        plan->writes[plan->write_count].value = value;
        plan->write_count++;
    }
}

static u32 igd9x_intel_plane_disable_value(u8 pipe)
{
    return (pipe == 0) ? 0x00000000UL : 0x01000000UL;
}

static u32 igd9x_intel_pipe_disable_value(void)
{
    return 0x00000000UL;
}

static igd9x_status_t igd9x_intel_apply_plan(const igd9x_intel_timing_plan_t *plan,
                                             const igd9x_mmio_ops_t *mmio)
{
    u16 i;
    igd9x_status_t status;
    const igd9x_intel_mmio_write_t *write;

    if (plan == 0 || mmio == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0; i < plan->write_count; ++i) {
        write = &plan->writes[i];
        status = igd9x_mmio_write32(mmio, write->reg, write->value);
        if (status != IGD9X_STATUS_OK) {
            return status;
        }
    }

    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_intel_identify(const igd9x_hw_info_t *hw,
                                    igd9x_intel_kind_t *kind_out)
{
    if (hw == 0 || kind_out == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }
    if (hw->vendor_id != IGD9X_INTEL_VENDOR_ID) {
        return IGD9X_STATUS_NOT_FOUND;
    }
    if ((hw->class_code & 0xFF000000UL) != IGD9X_INTEL_CLASS_DISPLAY) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    switch (hw->device_id) {
    case 0x0152U:
    case 0x0156U:
    case 0x015AU:
        *kind_out = IGD9X_INTEL_KIND_IVB_GT1;
        return IGD9X_STATUS_OK;
    case 0x0162U:
    case 0x0166U:
    case 0x016AU:
        *kind_out = IGD9X_INTEL_KIND_IVB_GT2;
        return IGD9X_STATUS_OK;
    default:
        *kind_out = IGD9X_INTEL_KIND_UNKNOWN;
        return IGD9X_STATUS_UNSUPPORTED;
    }
}

const char *igd9x_intel_kind_name(igd9x_intel_kind_t kind)
{
    switch (kind) {
    case IGD9X_INTEL_KIND_IVB_GT1:
        return "ivybridge-gt1";
    case IGD9X_INTEL_KIND_IVB_GT2:
        return "ivybridge-gt2";
    default:
        return "unknown";
    }
}

const char *igd9x_intel_mmio_reg_name(u32 reg)
{
    switch (reg) {
    case IGD9X_INTEL_MMIO_HTOTAL_A: return "HTOTAL_A";
    case IGD9X_INTEL_MMIO_HBLANK_A: return "HBLANK_A";
    case IGD9X_INTEL_MMIO_HSYNC_A: return "HSYNC_A";
    case IGD9X_INTEL_MMIO_VTOTAL_A: return "VTOTAL_A";
    case IGD9X_INTEL_MMIO_VBLANK_A: return "VBLANK_A";
    case IGD9X_INTEL_MMIO_VSYNC_A: return "VSYNC_A";
    case IGD9X_INTEL_MMIO_PIPEASRC: return "PIPEASRC";
    case IGD9X_INTEL_MMIO_HTOTAL_B: return "HTOTAL_B";
    case IGD9X_INTEL_MMIO_HBLANK_B: return "HBLANK_B";
    case IGD9X_INTEL_MMIO_HSYNC_B: return "HSYNC_B";
    case IGD9X_INTEL_MMIO_VTOTAL_B: return "VTOTAL_B";
    case IGD9X_INTEL_MMIO_VBLANK_B: return "VBLANK_B";
    case IGD9X_INTEL_MMIO_VSYNC_B: return "VSYNC_B";
    case IGD9X_INTEL_MMIO_PIPEBSRC: return "PIPEBSRC";
    case IGD9X_INTEL_MMIO_DPLL_A: return "DPLL_A";
    case IGD9X_INTEL_MMIO_DPLL_B: return "DPLL_B";
    case IGD9X_INTEL_MMIO_FP0: return "FP0";
    case IGD9X_INTEL_MMIO_FP1: return "FP1";
    case IGD9X_INTEL_MMIO_FPB0: return "FPB0";
    case IGD9X_INTEL_MMIO_FPB1: return "FPB1";
    case IGD9X_INTEL_MMIO_DSPA_CTRL: return "DSPA_CTRL";
    case IGD9X_INTEL_MMIO_DSPA_BASE: return "DSPA_BASE";
    case IGD9X_INTEL_MMIO_DSPA_STRIDE: return "DSPA_STRIDE";
    case IGD9X_INTEL_MMIO_DSPB_CTRL: return "DSPB_CTRL";
    case IGD9X_INTEL_MMIO_DSPB_BASE: return "DSPB_BASE";
    case IGD9X_INTEL_MMIO_DSPB_STRIDE: return "DSPB_STRIDE";
    case IGD9X_INTEL_MMIO_ADPA: return "ADPA";
    case IGD9X_INTEL_MMIO_DVOA: return "DVOA";
    case IGD9X_INTEL_MMIO_LVDS: return "LVDS";
    case IGD9X_INTEL_MMIO_PIPEA_CONF: return "PIPEACONF";
    case IGD9X_INTEL_MMIO_PIPEB_CONF: return "PIPEBCONF";
    default:
        return 0;
    }
}

igd9x_status_t igd9x_intel_build_legacy_pipe_plan(const igd9x_mode_t *mode,
                                                  u8 pipe,
                                                  igd9x_intel_timing_plan_t *plan)
{
    if (mode == 0 || plan == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }
    if (!igd9x_validate_mode(mode)) {
        return IGD9X_STATUS_UNSUPPORTED;
    }

    memset(plan, 0, sizeof(*plan));
    if (igd9x_intel_plane_control(mode, pipe) == 0UL) {
        return IGD9X_STATUS_UNSUPPORTED;
    }
    if (pipe == 0) {
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DSPA_CTRL, igd9x_intel_plane_disable_value(0));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_PIPEA_CONF, igd9x_intel_pipe_disable_value());
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_HTOTAL_A, igd9x_intel_pack_htotal(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_HBLANK_A, igd9x_intel_pack_hblank(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_HSYNC_A, igd9x_intel_pack_hsync(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_VTOTAL_A, igd9x_intel_pack_vtotal(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_VBLANK_A, igd9x_intel_pack_vblank(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_VSYNC_A, igd9x_intel_pack_vsync(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_PIPEASRC, igd9x_intel_pack_src(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DSPA_STRIDE, mode->pitch_bytes);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DSPA_BASE, 0UL);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DPLL_A, 0UL);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_FP0, 0UL);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_FP1, 0UL);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DSPA_CTRL, igd9x_intel_plane_control(mode, 0));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_PIPEA_CONF, igd9x_intel_pipe_control());
    } else {
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DSPB_CTRL, igd9x_intel_plane_disable_value(1));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_PIPEB_CONF, igd9x_intel_pipe_disable_value());
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_HTOTAL_B, igd9x_intel_pack_htotal(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_HBLANK_B, igd9x_intel_pack_hblank(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_HSYNC_B, igd9x_intel_pack_hsync(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_VTOTAL_B, igd9x_intel_pack_vtotal(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_VBLANK_B, igd9x_intel_pack_vblank(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_VSYNC_B, igd9x_intel_pack_vsync(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_PIPEBSRC, igd9x_intel_pack_src(mode));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DSPB_STRIDE, mode->pitch_bytes);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DSPB_BASE, 0UL);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DPLL_B, 0UL);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_FPB0, 0UL);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_FPB1, 0UL);
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_DSPB_CTRL, igd9x_intel_plane_control(mode, 1));
        igd9x_intel_plan_add(plan, IGD9X_INTEL_MMIO_PIPEB_CONF, igd9x_intel_pipe_control());
    }

    return IGD9X_STATUS_OK;
}

u16 igd9x_intel_plan_write_count(const igd9x_intel_timing_plan_t *plan)
{
    if (plan == 0) {
        return 0;
    }
    return plan->write_count;
}

const igd9x_intel_mmio_write_t *igd9x_intel_plan_write_at(const igd9x_intel_timing_plan_t *plan,
                                                          u16 index)
{
    if (plan == 0 || index >= plan->write_count) {
        return 0;
    }
    return &plan->writes[index];
}

igd9x_status_t igd9x_intel_backend_probe(void *context, const igd9x_hw_info_t *info)
{
    igd9x_intel_backend_state_t *state;
    igd9x_intel_kind_t kind;
    igd9x_status_t status;

    state = (igd9x_intel_backend_state_t *)context;
    if (state == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    status = igd9x_intel_identify(info, &kind);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    {
        const igd9x_mmio_ops_t *mmio;

        mmio = state->mmio;
        memset(state, 0, sizeof(*state));
        state->mmio = mmio;
    }
    state->pci.hw = *info;
    state->kind = kind;

    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_intel_backend_set_mode(void *context, const igd9x_mode_t *mode)
{
    igd9x_intel_backend_state_t *state;
    igd9x_status_t status;

    state = (igd9x_intel_backend_state_t *)context;
    if (state == 0 || mode == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    status = igd9x_intel_build_legacy_pipe_plan(mode, 0, &state->last_plan);
    if (status != IGD9X_STATUS_OK) {
        return status;
    }

    if (state->mmio != 0) {
        status = igd9x_intel_apply_plan(&state->last_plan, state->mmio);
        if (status != IGD9X_STATUS_OK) {
            return status;
        }

        if (state->kind == IGD9X_INTEL_KIND_IVB_GT1 ||
            state->kind == IGD9X_INTEL_KIND_IVB_GT2) {
            status = igd9x_intel_enable_internal_panel(state->mmio);
            if (status != IGD9X_STATUS_OK) {
                return status;
            }
        }
    }

    state->last_mode = *mode;
    state->has_mode = 1;

    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_intel_backend_restore(void *context)
{
    igd9x_intel_backend_state_t *state;

    state = (igd9x_intel_backend_state_t *)context;
    if (state == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    memset(&state->last_mode, 0, sizeof(state->last_mode));
    memset(&state->last_plan, 0, sizeof(state->last_plan));
    state->has_mode = 0;

    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_intel_backend_bind_mmio(void *context, const igd9x_mmio_ops_t *mmio)
{
    igd9x_intel_backend_state_t *state;

    state = (igd9x_intel_backend_state_t *)context;
    if (state == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    state->mmio = mmio;
    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_intel_backend_get_last_mmio_write(void *context, u32 offset, u32 *value_out)
{
    igd9x_intel_backend_state_t *state;

    state = (igd9x_intel_backend_state_t *)context;
    if (state == 0 || value_out == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }
    if (state->mmio == 0) {
        return IGD9X_STATUS_NOT_FOUND;
    }

    *value_out = igd9x_mmio_read32(state->mmio, offset);
    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_intel_backend_import_pci(void *context, const igd9x_pci_function_t *device)
{
    igd9x_intel_backend_state_t *state;

    state = (igd9x_intel_backend_state_t *)context;
    if (state == 0 || device == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    state->pci.hw.vendor_id = device->vendor_id;
    state->pci.hw.device_id = device->device_id;
    state->pci.hw.bus = device->bus;
    state->pci.hw.device = device->device;
    state->pci.hw.function = device->function;
    state->pci.hw.class_code = ((u32)device->class_code << 24) |
                                ((u32)device->subclass << 16) |
                                ((u32)device->prog_if << 8) |
                                (u32)device->revision_id;
    state->pci.mmio_base = device->bar0;
    state->pci.mmio_size = device->bar0_size;

    return IGD9X_STATUS_OK;
}

const igd9x_hw_ops_t igd9x_intel_backend_ops = {
    "intel-hd-4000-backend",
    &g_intel_backend_state,
    igd9x_intel_backend_probe,
    igd9x_intel_backend_set_mode,
    igd9x_intel_backend_restore
};
