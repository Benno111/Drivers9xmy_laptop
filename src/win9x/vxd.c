#include "../../include/igd9x_driver.h"

static const igd9x_hw_ops_t g_vxd_stub_hw_ops = {
    "win9x-vxd-stub",
    0,
    0,
    0,
    0
};

const igd9x_hw_ops_t *igd9x_win9x_vxd_default_hw_ops(void)
{
    return &g_vxd_stub_hw_ops;
}
