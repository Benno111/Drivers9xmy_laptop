#include <string.h>
#include "../../include/igd9x_pci.h"

typedef struct igd9x_win9x_pci_mock {
    igd9x_pci_function_t device;
    u32 config[64];
    u8 present;
    u8 bar0_probe_active;
} igd9x_win9x_pci_mock_t;

static igd9x_win9x_pci_mock_t g_win9x_pci_mock;

static u32 igd9x_win9x_pci_build_class_dword(const igd9x_pci_function_t *device)
{
    return ((u32)device->class_code << 24) |
           ((u32)device->subclass << 16) |
           ((u32)device->prog_if << 8) |
           (u32)device->revision_id;
}

static u32 igd9x_win9x_pci_read32_impl(void *context, u8 bus, u8 device, u8 function, u8 offset)
{
    igd9x_win9x_pci_mock_t *mock;

    mock = (igd9x_win9x_pci_mock_t *)context;
    if (mock == 0 || !mock->present) {
        return 0xFFFFFFFFUL;
    }
    if (bus != mock->device.bus ||
        device != mock->device.device ||
        function != mock->device.function) {
        return 0xFFFFFFFFUL;
    }
    if ((offset & 3U) != 0U) {
        return 0xFFFFFFFFUL;
    }

    if (offset == 0x10U && mock->bar0_probe_active) {
        return (~(mock->device.bar0_size - 1UL) & 0xFFFFFFF0UL);
    }

    return mock->config[offset / 4U];
}

static void igd9x_win9x_pci_write32_impl(void *context, u8 bus, u8 device, u8 function, u8 offset, u32 value)
{
    igd9x_win9x_pci_mock_t *mock;

    mock = (igd9x_win9x_pci_mock_t *)context;
    if (mock == 0 || !mock->present) {
        return;
    }
    if (bus != mock->device.bus ||
        device != mock->device.device ||
        function != mock->device.function) {
        return;
    }
    if ((offset & 3U) != 0U) {
        return;
    }

    if (offset == 0x10U) {
        if (value == 0xFFFFFFFFUL) {
            mock->bar0_probe_active = 1;
            return;
        }
        mock->bar0_probe_active = 0;
    }

    mock->config[offset / 4U] = value;
}

const igd9x_pci_ops_t igd9x_win9x_pci_ops = {
    &g_win9x_pci_mock,
    igd9x_win9x_pci_read32_impl,
    igd9x_win9x_pci_write32_impl
};

igd9x_status_t igd9x_win9x_pci_bind_mock_device(const igd9x_pci_function_t *device)
{
    if (device == 0) {
        return IGD9X_STATUS_INVALID_ARGUMENT;
    }

    memset(&g_win9x_pci_mock, 0, sizeof(g_win9x_pci_mock));
    g_win9x_pci_mock.device = *device;
    g_win9x_pci_mock.present = 1;
    g_win9x_pci_mock.config[0] = ((u32)device->device_id << 16) | (u32)device->vendor_id;
    g_win9x_pci_mock.config[2] = igd9x_win9x_pci_build_class_dword(device);
    g_win9x_pci_mock.config[4] = 0x00000000UL;
    g_win9x_pci_mock.config[0x10U / 4U] = device->bar0;
    return IGD9X_STATUS_OK;
}

const igd9x_pci_function_t *igd9x_win9x_pci_mock_device(void)
{
    if (!g_win9x_pci_mock.present) {
        return 0;
    }
    return &g_win9x_pci_mock.device;
}

