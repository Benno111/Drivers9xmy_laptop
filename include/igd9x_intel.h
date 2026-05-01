#ifndef IGD9X_INTEL_H
#define IGD9X_INTEL_H

#include "igd9x_driver.h"
#include "igd9x_pci.h"
#include "igd9x_mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum igd9x_intel_kind {
    IGD9X_INTEL_KIND_UNKNOWN = 0,
    IGD9X_INTEL_KIND_IVB_GT1,
    IGD9X_INTEL_KIND_IVB_GT2
} igd9x_intel_kind_t;

typedef struct igd9x_intel_pci_info {
    igd9x_hw_info_t hw;
    u32 mmio_base;
    u32 mmio_size;
    u32 stolen_kb;
    u32 gtt_kb;
} igd9x_intel_pci_info_t;

typedef struct igd9x_intel_mmio_write {
    u32 reg;
    u32 value;
} igd9x_intel_mmio_write_t;

typedef struct igd9x_intel_timing_plan {
    u16 write_count;
    igd9x_intel_mmio_write_t writes[16];
} igd9x_intel_timing_plan_t;

typedef struct igd9x_intel_backend_state {
    igd9x_intel_pci_info_t pci;
    igd9x_intel_kind_t kind;
    const igd9x_mmio_ops_t *mmio;
    igd9x_mode_t last_mode;
    igd9x_intel_timing_plan_t last_plan;
    u8 has_mode;
} igd9x_intel_backend_state_t;

enum igd9x_intel_mmio_reg {
    IGD9X_INTEL_MMIO_HTOTAL_A = 0x60000UL,
    IGD9X_INTEL_MMIO_HBLANK_A = 0x60004UL,
    IGD9X_INTEL_MMIO_HSYNC_A = 0x60008UL,
    IGD9X_INTEL_MMIO_VTOTAL_A = 0x6000CUL,
    IGD9X_INTEL_MMIO_VBLANK_A = 0x60010UL,
    IGD9X_INTEL_MMIO_VSYNC_A = 0x60014UL,
    IGD9X_INTEL_MMIO_PIPEASRC = 0x6001CUL,
    IGD9X_INTEL_MMIO_HTOTAL_B = 0x61000UL,
    IGD9X_INTEL_MMIO_HBLANK_B = 0x61004UL,
    IGD9X_INTEL_MMIO_HSYNC_B = 0x61008UL,
    IGD9X_INTEL_MMIO_VTOTAL_B = 0x6100CUL,
    IGD9X_INTEL_MMIO_VBLANK_B = 0x61010UL,
    IGD9X_INTEL_MMIO_VSYNC_B = 0x61014UL,
    IGD9X_INTEL_MMIO_PIPEBSRC = 0x6101CUL,
    IGD9X_INTEL_MMIO_DPLL_A = 0x06014UL,
    IGD9X_INTEL_MMIO_DPLL_B = 0x06018UL,
    IGD9X_INTEL_MMIO_FP0 = 0x06040UL,
    IGD9X_INTEL_MMIO_FP1 = 0x06044UL,
    IGD9X_INTEL_MMIO_FPB0 = 0x06048UL,
    IGD9X_INTEL_MMIO_FPB1 = 0x0604CUL,
    IGD9X_INTEL_MMIO_DSPA_CTRL = 0x70180UL,
    IGD9X_INTEL_MMIO_DSPA_BASE = 0x70184UL,
    IGD9X_INTEL_MMIO_DSPA_STRIDE = 0x70188UL,
    IGD9X_INTEL_MMIO_DSPB_CTRL = 0x71180UL,
    IGD9X_INTEL_MMIO_DSPB_BASE = 0x71184UL,
    IGD9X_INTEL_MMIO_DSPB_STRIDE = 0x71188UL,
    IGD9X_INTEL_MMIO_ADPA = 0x61100UL,
    IGD9X_INTEL_MMIO_DVOA = 0x61120UL,
    IGD9X_INTEL_MMIO_LVDS = 0x61180UL,
    IGD9X_INTEL_MMIO_PIPEA_CONF = 0x70008UL,
    IGD9X_INTEL_MMIO_PIPEB_CONF = 0x71008UL
};

#define IGD9X_INTEL_DSPCNTR_ENABLE      0x80000000UL
#define IGD9X_INTEL_DSPCNTR_GAMMA       0x40000000UL
#define IGD9X_INTEL_DSPCNTR_FMT_16BPP   0x14000000UL
#define IGD9X_INTEL_DSPCNTR_FMT_32BPP   0x1C000000UL
#define IGD9X_INTEL_DSPCNTR_SEL_PIPE_A  0x00000000UL
#define IGD9X_INTEL_DSPCNTR_SEL_PIPE_B  0x01000000UL

#define IGD9X_INTEL_PIPECONF_ENABLE     0x80000000UL
#define IGD9X_INTEL_PIPECONF_GAMMA      0x01000000UL

igd9x_status_t igd9x_intel_identify(const igd9x_hw_info_t *hw,
                                    igd9x_intel_kind_t *kind_out);
const char *igd9x_intel_kind_name(igd9x_intel_kind_t kind);
const char *igd9x_intel_mmio_reg_name(u32 reg);
igd9x_status_t igd9x_intel_build_legacy_pipe_plan(const igd9x_mode_t *mode,
                                                  u8 pipe,
                                                  igd9x_intel_timing_plan_t *plan);
u16 igd9x_intel_plan_write_count(const igd9x_intel_timing_plan_t *plan);
const igd9x_intel_mmio_write_t *igd9x_intel_plan_write_at(const igd9x_intel_timing_plan_t *plan,
                                                          u16 index);
igd9x_status_t igd9x_intel_backend_bind_mmio(void *context, const igd9x_mmio_ops_t *mmio);
igd9x_status_t igd9x_intel_backend_get_last_mmio_write(void *context, u32 offset, u32 *value_out);
igd9x_status_t igd9x_intel_backend_import_pci(void *context, const igd9x_pci_function_t *device);

igd9x_status_t igd9x_intel_backend_probe(void *context, const igd9x_hw_info_t *info);
igd9x_status_t igd9x_intel_backend_set_mode(void *context, const igd9x_mode_t *mode);
igd9x_status_t igd9x_intel_backend_restore(void *context);

extern const igd9x_hw_ops_t igd9x_intel_backend_ops;

#ifdef __cplusplus
}
#endif

#endif
