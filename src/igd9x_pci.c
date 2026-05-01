#include "../include/igd9x_pci.h"
#include "../include/igd9x_intel.h"

#define IGD9X_PCI_VENDOR_INTEL 0x8086U
#define IGD9X_PCI_CLASS_DISPLAY 0x03U
#define IGD9X_PCI_SUBCLASS_VGA 0x00U

static u32 igd9x_pci_mask_bar_size(u32 value)
{
    return (~(value & 0xFFFFFFF0UL)) + 1UL;
}

static igd9x_status_t igd9x_pci_probe_bar0(const igd9x_pci_ops_t *ops,
                                           u8 bus,
                                           u8 device,
                                           u8 function,
                                           u32 *base_out,
                                           u32 *size_out)
{
    u32 original;
    u32 sized;
    igd9x_status_t status;

    if (base_out == 0 || size_out == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    if (ops == 0 || ops->read32 == 0) {
        *base_out = 0;
        *size_out = 0;
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    original = ops->read32(ops->context, bus, device, function, 0x10U);
    *base_out = original & 0xFFFFFFF0UL;
    *size_out = 0;

    if (ops->write32 == 0) {
        return IGD9X_STATUS_OK;
    }

    ops->write32(ops->context, bus, device, function, 0x10U, 0xFFFFFFFFUL);
    sized = ops->read32(ops->context, bus, device, function, 0x10U);
    ops->write32(ops->context, bus, device, function, 0x10U, original);

    if (sized == 0 || sized == 0xFFFFFFFFUL) {
        return IGD9X_STATUS_OK;
    }

    *size_out = igd9x_pci_mask_bar_size(sized);
    status = IGD9X_STATUS_OK;
    return status;
}

static igd9x_status_t igd9x_pci_read_function(const igd9x_pci_ops_t *ops,
                                              u8 bus,
                                              u8 device,
                                              u8 function,
                                              igd9x_pci_function_t *device_out)
{
    u32 id_dword;
    u32 class_dword;

    if (ops == 0 || ops->read32 == 0 || device_out == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    id_dword = ops->read32(ops->context, bus, device, function, 0x00U);
    if (id_dword == 0xFFFFFFFFUL || id_dword == 0x00000000UL) {
        return IGD9X_STATUS_NOT_FOUND;
    }

    class_dword = ops->read32(ops->context, bus, device, function, 0x08U);

    device_out->bus = bus;
    device_out->device = device;
    device_out->function = function;
    device_out->vendor_id = (u16)(id_dword & 0xFFFFU);
    device_out->device_id = (u16)((id_dword >> 16) & 0xFFFFU);
    device_out->revision_id = (u8)(class_dword & 0xFFU);
    device_out->prog_if = (u8)((class_dword >> 8) & 0xFFU);
    device_out->subclass = (u8)((class_dword >> 16) & 0xFFU);
    device_out->class_code = (u8)((class_dword >> 24) & 0xFFU);
    device_out->bar0 = 0;
    device_out->bar0_size = 0;

    return IGD9X_STATUS_OK;
}

igd9x_status_t igd9x_pci_discover_intel_display(const igd9x_pci_ops_t *ops,
                                                igd9x_pci_function_t *device_out)
{
    u16 bus;
    u8 device;
    u8 function;

    if (ops == 0 || device_out == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    for (bus = 0; bus < 256U; ++bus) {
        for (device = 0; device < 32U; ++device) {
            for (function = 0; function < 8U; ++function) {
                igd9x_pci_function_t candidate;
                igd9x_status_t status;

                status = igd9x_pci_read_function(ops, (u8)bus, device, function, &candidate);
                if (status != IGD9X_STATUS_OK) {
                    continue;
                }
                if (candidate.vendor_id == IGD9X_PCI_VENDOR_INTEL &&
                    candidate.class_code == IGD9X_PCI_CLASS_DISPLAY &&
                    candidate.subclass == IGD9X_PCI_SUBCLASS_VGA) {
                    igd9x_hw_info_t hw;
                    igd9x_intel_kind_t kind;
                    igd9x_status_t bar_status;

                    if (igd9x_pci_function_to_hw_info(&candidate, &hw) != IGD9X_STATUS_OK) {
                        continue;
                    }
                    if (igd9x_intel_identify(&hw, &kind) != IGD9X_STATUS_OK) {
                        continue;
                    }
                    bar_status = igd9x_pci_probe_bar0(ops, (u8)bus, device, function,
                                                      &candidate.bar0,
                                                      &candidate.bar0_size);
                    if (bar_status != IGD9X_STATUS_OK) {
                        continue;
                    }
                    (void)kind;
                    *device_out = candidate;
                    return IGD9X_STATUS_OK;
                }
            }
        }
    }

    return IGD9X_STATUS_NOT_FOUND;
}

igd9x_status_t igd9x_pci_function_to_hw_info(const igd9x_pci_function_t *device,
                                             igd9x_hw_info_t *hw_out)
{
    if (device == 0 || hw_out == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    hw_out->vendor_id = device->vendor_id;
    hw_out->device_id = device->device_id;
    hw_out->bus = device->bus;
    hw_out->device = device->device;
    hw_out->function = device->function;
    hw_out->class_code = ((u32)device->class_code << 24) |
                         ((u32)device->subclass << 16) |
                         ((u32)device->prog_if << 8) |
                         (u32)device->revision_id;

    return IGD9X_STATUS_OK;
}
