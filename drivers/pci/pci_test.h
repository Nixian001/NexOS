#include <stdint.h>
#include "pci.h"
#include "../display/display.h"

// PCI class/subclass/progif codes for USB controllers
#define PCI_CLASS_SERIAL      0x0C
#define PCI_SUBCLASS_USB      0x03
#define PROGIF_UHCI           0x00
#define PROGIF_OHCI           0x10
#define PROGIF_EHCI           0x20
#define PROGIF_XHCI           0x30

void scan_usb_controllers(void);
void dump_pci_all(void);