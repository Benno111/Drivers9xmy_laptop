#ifndef IGD9X_WIN9X_MMIO_H
#define IGD9X_WIN9X_MMIO_H

#include "igd9x_mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const igd9x_mmio_ops_t igd9x_win9x_mmio_ops;

igd9x_status_t igd9x_win9x_mmio_bind(u32 base_phys, u32 size);
void *igd9x_win9x_mmio_context(void);

#ifdef __cplusplus
}
#endif

#endif

