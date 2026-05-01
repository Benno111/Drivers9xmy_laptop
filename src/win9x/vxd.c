#include "../../include/igd9x_intel.h"

const igd9x_hw_ops_t *igd9x_win9x_vxd_default_hw_ops(void)
{
    return &igd9x_intel_backend_ops;
}
