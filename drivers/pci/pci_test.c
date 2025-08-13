#include "pci_test.h"

void scan_usb_controllers(void) {
    print_string("Scanning for USB controllers...\n");

    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t device = 0; device < 32; device++) {
            for (uint32_t function = 0; function < 8; function++) {

                pci_dev_t dev = { .bits = 0 };

                dev.bus_num = bus;
                dev.device_num = device;
                dev.function_num = function;
                dev.enable = 1;

                uint32_t vendor_id = pci_read(dev, 0x00);
                if (vendor_id == 0xFFFFFFFF) continue; // No device found

                uint32_t class_reg = pci_read(dev, 0x08);
                uint8_t class_code = pci_read(dev, PCI_CLASS);
                uint8_t subclass = pci_read(dev, PCI_SUBCLASS);
                uint8_t prog_if = pci_read(dev, PCI_PROG_IF);

                if (class_code == PCI_CLASS_SERIAL && subclass == PCI_SUBCLASS_USB) {
                    print_string("Found USB controller:\n");

                    switch (prog_if) {
                        case PROGIF_UHCI:
                            print_string("UHCI (USB 1.1)\n");
                            break;
                        case PROGIF_OHCI:
                            print_string("OHCI (USB 1.1)\n");
                            break;
                        case PROGIF_EHCI:
                            print_string("EHCI (USB 2.0)\n");
                            break;
                        case PROGIF_XHCI:
                            print_string("xHCI (USB 3.x)\n");
                            break;
                        default: {
                            print_string("Unknown type (ProgIF: 0x");
                            char buf[5];
                            itoa(prog_if, buf, 16);
                            print_string(buf);
                            print_string(")\n");
                            break;
                        }
                    }
                }
            }
        }
    }
}

void dump_pci_all(void) {
    print_string("=== PCI DEVICE LIST ===\n");

    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t device = 0; device < 32; device++) {
            for (uint32_t func = 0; func < 8; func++) {
                pci_dev_t dev = { .bits = 0 };
                dev.bus_num      = bus;
                dev.device_num   = device;
                dev.function_num = func;
                dev.enable       = 1;

                uint32_t vendor_device = pci_read(dev, 0x00);
                if (vendor_device == 0xFFFFFFFF) continue;

                uint16_t vendor_id = vendor_device & 0xFFFF;
                uint16_t device_id = (vendor_device >> 16) & 0xFFFF;

                uint32_t class_reg = pci_read(dev, 0x08);
                uint8_t prog_if    = (class_reg >> 8)  & 0xFF;
                uint8_t subclass   = (class_reg >> 16) & 0xFF;
                uint8_t class_code = (class_reg >> 24) & 0xFF;

                print_string("Bus ");
                print_int(bus);
                print_string(", Dev ");
                print_int(device);
                print_string(", Func ");
                print_int(func);

                print_string(": Vendor ");
                print_int(vendor_id);
                print_string(", Device ");
                print_int(device_id);

                print_string("  Class ");
                print_int(class_code);
                print_string(", Subclass ");
                print_int(subclass);
                // print_string(", ProgIF 0x");
                // print_int(prog_if);

                print_nl();
            }
        }
    }

    print_string("========================\n");
}