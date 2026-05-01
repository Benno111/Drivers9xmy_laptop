#include <stdio.h>
#include "../include/igd9x_driver.h"
#include "../include/igd9x_pci.h"
#include "../include/igd9x_intel.h"
#include "../include/igd9x_win9x_exports.h"
#include "../include/igd9x_win9x_pci.h"
#include "../include/igd9x_win9x_mmio.h"

static void print_mode(const igd9x_mode_t *mode, void *context)
{
    char name[32];

    (void)context;
    if (igd9x_driver_describe_mode(mode, name, (u16)sizeof(name)) != 0) {
        printf("%s pitch=%u clock=%lu kHz %s%s\n",
               name,
               (unsigned int)mode->pitch_bytes,
               (unsigned long)mode->pixel_clock_khz,
               mode->preferred ? "preferred" : "",
               mode->interlaced ? " interlaced" : "");
    }
}

int main(void)
{
    igd9x_pci_function_t device;
    igd9x_pci_function_t discovered;
    const igd9x_mode_t *mode;
    char name[32];
    igd9x_intel_backend_state_t *intel_state;
    const igd9x_intel_mmio_write_t *write;
    igd9x_status_t status;
    u16 i;

    device.bus = 0;
    device.device = 2;
    device.function = 0;
    device.vendor_id = 0x8086U;
    device.device_id = 0x0162U;
    device.class_code = 0x03U;
    device.subclass = 0x00U;
    device.prog_if = 0x00U;
    device.revision_id = 0x09U;
    device.bar0 = 0xF0000000UL;
    device.bar0_size = 0x00080000UL;

    status = igd9x_win9x_pci_bind_mock_device(&device);
    if (status != IGD9X_STATUS_OK) {
        printf("pci mock bind failed: %u\n", (unsigned int)status);
        return 1;
    }

    status = igd9x_pci_discover_intel_display(&igd9x_win9x_pci_ops, &discovered);
    if (status != IGD9X_STATUS_OK) {
        printf("pci discovery failed: %u\n", (unsigned int)status);
        return 1;
    }

    status = igd9x_win9x_driver_startup();
    if (status != IGD9X_STATUS_OK) {
        printf("driver startup failed: %u\n", (unsigned int)status);
        return 1;
    }

    status = igd9x_win9x_driver_discover();
    if (status != IGD9X_STATUS_OK) {
        printf("driver discover failed: %u\n", (unsigned int)status);
        igd9x_win9x_driver_shutdown();
        return 1;
    }

    status = igd9x_win9x_driver_attach();
    if (status != IGD9X_STATUS_OK) {
        printf("driver attach failed: %u\n", (unsigned int)status);
        igd9x_win9x_driver_shutdown();
        return 1;
    }

    printf("IGD9X Win9x layout: %u modes\n", (unsigned int)igd9x_driver_mode_count());
    printf("Discovered PCI: bus %u device %u function %u device 0x%04x BAR0 0x%08lx size 0x%08lx\n",
           (unsigned int)discovered.bus,
           (unsigned int)discovered.device,
           (unsigned int)discovered.function,
           (unsigned int)discovered.device_id,
           (unsigned long)discovered.bar0,
           (unsigned long)discovered.bar0_size);
    if (igd9x_win9x_driver_state() != 0) {
        mode = igd9x_win9x_driver_state()->driver.current_mode;
    } else {
        mode = 0;
    }
    if (mode != 0 &&
        igd9x_driver_describe_mode(mode, name, (u16)sizeof(name)) != 0) {
        printf("Native mode: %s\n", name);
    }

    status = igd9x_win9x_driver_enumerate_modes(print_mode, 0);
    if (status != IGD9X_STATUS_OK) {
        printf("enumeration failed: %u\n", (unsigned int)status);
        igd9x_win9x_driver_shutdown();
        return 1;
    }

    status = igd9x_win9x_driver_set_mode_request(1366, 768, 32, 60);
    if (status == IGD9X_STATUS_OK) {
        if (igd9x_win9x_driver_state() != 0) {
            mode = igd9x_win9x_driver_state()->driver.current_mode;
        } else {
            mode = 0;
        }
        if (mode != 0) {
            printf("Active mode now: %s\n",
                   igd9x_driver_describe_mode(mode, name, (u16)sizeof(name)));
        }
        intel_state = (igd9x_intel_backend_state_t *)igd9x_intel_backend_ops.context;
        printf("Legacy pipe plan writes: %u\n",
               (unsigned int)igd9x_intel_plan_write_count(&intel_state->last_plan));
        for (i = 0; i < igd9x_intel_plan_write_count(&intel_state->last_plan); ++i) {
            write = igd9x_intel_plan_write_at(&intel_state->last_plan, i);
            if (write != 0) {
                printf("  %s = 0x%08lx\n",
                       igd9x_intel_mmio_reg_name(write->reg) != 0 ?
                       igd9x_intel_mmio_reg_name(write->reg) : "UNKNOWN",
                       (unsigned long)write->value);
            }
        }
        printf("MMIO pipe enable: 0x%08lx\n",
               (unsigned long)igd9x_mmio_read32(&igd9x_win9x_mmio_ops,
                                                IGD9X_INTEL_MMIO_PIPEA_CONF));
    } else {
        printf("mode request failed: %u\n", (unsigned int)status);
    }

    igd9x_win9x_driver_shutdown();

    return 0;
}
