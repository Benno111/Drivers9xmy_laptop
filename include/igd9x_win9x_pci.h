#ifndef IGD9X_WIN9X_PCI_H
#define IGD9X_WIN9X_PCI_H

#include "igd9x_pci.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const igd9x_pci_ops_t igd9x_win9x_pci_ops;

igd9x_status_t igd9x_win9x_pci_bind_mock_device(const igd9x_pci_function_t *device);
const igd9x_pci_function_t *igd9x_win9x_pci_mock_device(void);

#ifdef __cplusplus
}
#endif

#endif

