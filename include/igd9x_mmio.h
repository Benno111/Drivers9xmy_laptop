#ifndef IGD9X_MMIO_H
#define IGD9X_MMIO_H

#include "igd9x.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct igd9x_mmio_ops {
    void *context;
    u32 (*read32)(void *context, u32 offset);
    void (*write32)(void *context, u32 offset, u32 value);
} igd9x_mmio_ops_t;

igd9x_status_t igd9x_mmio_write32(const igd9x_mmio_ops_t *ops, u32 offset, u32 value);
u32 igd9x_mmio_read32(const igd9x_mmio_ops_t *ops, u32 offset);

#ifdef __cplusplus
}
#endif

#endif

