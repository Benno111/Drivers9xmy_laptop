#include "../../include/igd9x_win9x_driver.h"

int IGD9X_WIN9X_ENTRY DriverEntry(void)
{
    igd9x_status_t status;

    status = igd9x_win9x_driver_startup();
    if (status != IGD9X_STATUS_OK) {
        return -1;
    }

    status = igd9x_win9x_driver_discover();
    if (status != IGD9X_STATUS_OK) {
        igd9x_win9x_driver_shutdown();
        return -2;
    }

    status = igd9x_win9x_driver_attach();
    if (status != IGD9X_STATUS_OK) {
        igd9x_win9x_driver_shutdown();
        return -3;
    }

    status = igd9x_win9x_driver_set_native_mode();
    if (status != IGD9X_STATUS_OK) {
        igd9x_win9x_driver_shutdown();
        return -4;
    }

    return 0;
}
