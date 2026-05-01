#include <string.h>
#include "../../include/igd9x_mmio.h"

#define IGD9X_WIN9X_MMIO_BYTES 0x80000UL

typedef struct igd9x_win9x_mmio_region {
    u8 bytes[IGD9X_WIN9X_MMIO_BYTES];
    u32 base_phys;
    u32 size;
} igd9x_win9x_mmio_region_t;

static u32 igd9x_win9x_mmio_read32_impl(void *context, u32 offset)
{
    igd9x_win9x_mmio_region_t *region;
    u32 value;

    region = (igd9x_win9x_mmio_region_t *)context;
    if (region == 0 || offset + 4UL > region->size) {
        return 0;
    }

    memcpy(&value, &region->bytes[offset], sizeof(value));
    return value;
}

static void igd9x_win9x_mmio_write32_impl(void *context, u32 offset, u32 value)
{
    igd9x_win9x_mmio_region_t *region;

    region = (igd9x_win9x_mmio_region_t *)context;
    if (region == 0 || offset + 4UL > region->size) {
        return;
    }

    memcpy(&region->bytes[offset], &value, sizeof(value));
}

static igd9x_win9x_mmio_region_t g_win9x_mmio_region;

const igd9x_mmio_ops_t igd9x_win9x_mmio_ops = {
    &g_win9x_mmio_region,
    igd9x_win9x_mmio_read32_impl,
    igd9x_win9x_mmio_write32_impl
};

igd9x_status_t igd9x_win9x_mmio_bind(u32 base_phys, u32 size)
{
    if (size == 0 || size > IGD9X_WIN9X_MMIO_BYTES) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    memset(&g_win9x_mmio_region, 0, sizeof(g_win9x_mmio_region));
    g_win9x_mmio_region.base_phys = base_phys;
    g_win9x_mmio_region.size = size;
    return IGD9X_STATUS_OK;
}

void *igd9x_win9x_mmio_context(void)
{
    return &g_win9x_mmio_region;
}

