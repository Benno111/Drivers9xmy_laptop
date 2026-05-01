#include "../include/igd9x_mmio.h"

igd9x_status_t igd9x_mmio_write32(const igd9x_mmio_ops_t *ops, u32 offset, u32 value)
{
    if (ops == 0 || ops->write32 == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    ops->write32(ops->context, offset, value);
    return IGD9X_STATUS_OK;
}

u32 igd9x_mmio_read32(const igd9x_mmio_ops_t *ops, u32 offset)
{
    if (ops == 0 || ops->read32 == 0) {
        return 0;
    }

    return ops->read32(ops->context, offset);
}

