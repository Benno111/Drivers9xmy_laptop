#ifndef IGD9X_PCI_H
#define IGD9X_PCI_H

#include "igd9x.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct igd9x_pci_ops {
    void *context;
    u32 (*read32)(void *context, u8 bus, u8 device, u8 function, u8 offset);
    void (*write32)(void *context, u8 bus, u8 device, u8 function, u8 offset, u32 value);
} igd9x_pci_ops_t;

typedef struct igd9x_pci_function {
    u8 bus;
    u8 device;
    u8 function;
    u16 vendor_id;
    u16 device_id;
    u8 class_code;
    u8 subclass;
    u8 prog_if;
    u8 revision_id;
    u32 bar0;
    u32 bar0_size;
} igd9x_pci_function_t;

igd9x_status_t igd9x_pci_discover_intel_display(const igd9x_pci_ops_t *ops,
                                                igd9x_pci_function_t *device_out);
igd9x_status_t igd9x_pci_function_to_hw_info(const igd9x_pci_function_t *device,
                                             igd9x_hw_info_t *hw_out);

#ifdef __cplusplus
}
#endif

#endif

