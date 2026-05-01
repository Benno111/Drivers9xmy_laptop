#include "../../include/igd9x_win9x_driver.h"

int IGD9X_WIN9X_ENTRY DriverEntry(void)
{
    return (igd9x_win9x_driver_startup() == IGD9X_STATUS_OK) ? 0 : -1;
}

