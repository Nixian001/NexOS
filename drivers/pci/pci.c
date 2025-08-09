#include "pci.h"
#include "pci_driver.h"
#include "pci_registry.h"
#include "../display/display.h"

static void pci_visit(uint32_t bus, uint32_t device, uint32_t function) {
    uint32_t id = PCI_MAKE_ID(bus, device, function);

    pci_device_info info;
    info.vendor_id = pci_read_16(id, PCI_CONFIG_VENDOR_ID);
    if (info.vendor_id == 0xFFFF) {
        return; // Device not present
    }

    info.device_id = pci_read_16(id, PCI_CONFIG_DEVICE_ID);
    info.prog_intf = pci_read_8(id, PCI_CONFIG_PROG_INTF);
    info.subclass = pci_read_8(id, PCI_CONFIG_SUBCLASS);
    info.class_code = pci_read_8(id, PCI_CONFIG_CLASS_CODE);

    // Print device information
    print_string("PCI Device -> ");
    print_int(bus);
    print_string(":");
    print_int(device);
    print_string(":");
    print_int(function);
    print_string(" 0x");
    print_int(info.vendor_id);
    print_string("/0x");
    print_int(info.device_id);
    print_string(": ");
    print_string(pci_class_name(info.class_code, info.subclass, info.prog_intf));
    print_nl();

    const pci_driver *driver = pci_driver_table;

    while (driver->init) {
        driver->init(id, &info);
        ++driver;
    }
}

void pci_init() {
    print_string("Initializing PCI subsystem.\n");

    // Read the PCI configuration space
    for (uint32_t bus = 0; bus < 256; ++bus) {
        for (uint32_t device = 0; device < 32; ++device) {

            uint32_t base_id = PCI_MAKE_ID(bus, device, 0);
            uint8_t header_type = pci_read_8(base_id, PCI_CONFIG_HEADER_TYPE);
            uint32_t function_count = header_type & PCI_TYPE_MULTIFUNC ? 8 : 1;

            for (uint32_t function = 0; function < function_count; ++function) {
                pci_visit(bus, device, function);
            }
        }
    }

    print_string("PCI initialization complete.\n");
}