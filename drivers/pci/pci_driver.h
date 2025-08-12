#pragma once

#include <stdint.h>

#define PCI_MAKE_ID(bus, dev, func)     ((bus) << 16) | ((dev) << 11) | ((func) << 8)

// I/O Ports
#define PCI_CONFIG_ADDR                 0xCF8
#define PCI_CONFIG_DATA                 0xCFC

// Header Type
#define PCI_TYPE_MULTIFUNC              0x80
#define PCI_TYPE_GENERIC                0x00
#define PCI_TYPE_PCI_BRIDGE             0x01
#define PCI_TYPE_CARDBUS_BRIDGE         0x02

// PCI Configuration Registers
#define PCI_CONFIG_VENDOR_ID            0x00
#define PCI_CONFIG_DEVICE_ID            0x02
#define PCI_CONFIG_COMMAND              0x04
#define PCI_CONFIG_STATUS               0x06
#define PCI_CONFIG_REVISION_ID          0x08
#define PCI_CONFIG_PROG_INTF            0x09
#define PCI_CONFIG_SUBCLASS             0x0A
#define PCI_CONFIG_CLASS_CODE           0x0B
#define PCI_CONFIG_CACHELINE_SIZE       0x0C
#define PCI_CONFIG_LATENCY              0x0D
#define PCI_CONFIG_HEADER_TYPE          0x0E
#define PCI_CONFIG_BIST                 0x0F

// Type 0x00 (Generic) Configuration Registers
#define PCI_CONFIG_BAR0                 0x10
#define PCI_CONFIG_BAR1                 0x14
#define PCI_CONFIG_BAR2                 0x18
#define PCI_CONFIG_BAR3                 0x1C
#define PCI_CONFIG_BAR4                 0x20
#define PCI_CONFIG_BAR5                 0x24
#define PCI_CONFIG_CARDBUS_CIS          0x28
#define PCI_CONFIG_SUBSYSTEM_VENDOR_ID  0x2C
#define PCI_CONFIG_SUBSYSTEM_DEVICE_ID  0x2E
#define PCI_CONFIG_EXPANSION_ROM        0x30
#define PCI_CONFIG_CAPABILITIES         0x34
#define PCI_CONFIG_INTERRUPT_LINE       0x3C
#define PCI_CONFIG_INTERRUPT_PIN        0x3D
#define PCI_CONFIG_MIN_GRANT            0x3E
#define PCI_CONFIG_MAX_LATENCY          0x3F

// PCI BAR
#define PCI_BAR_IO                      0x01
#define PCI_BAR_LOWMEM                  0x02
#define PCI_BAR_64                      0x04
#define PCI_BAR_PREFETCH                0x08

typedef struct pci_bar {
    union {
        void *address;
        uint16_t port;
    } u;
    uint64_t size;
    uint32_t flags; // Flags for the BAR (I/O, Memory, etc.)
} pci_bar;

typedef struct pci_device_info {
    uint16_t vendor_id;        // Vendor ID
    uint16_t device_id;        // Device ID
    uint8_t class_code;        // Class code
    uint8_t subclass;          // Subclass code
    uint8_t prog_intf;         // Programming interface
} pci_device_info;

typedef struct pci_driver {
    void (*init)(uint32_t id, pci_device_info *info);
} pci_driver;

extern const pci_driver pci_driver_table[];

uint8_t  pci_read_8 (uint32_t id, uint32_t reg);
uint16_t pci_read_16(uint32_t id, uint32_t reg);
uint32_t pci_read_32(uint32_t id, uint32_t reg);

void    pci_write_8 (uint32_t id, uint32_t reg, uint8_t  data);
void    pci_write_16(uint32_t id, uint32_t reg, uint16_t data);
void    pci_write_32(uint32_t id, uint32_t reg, uint32_t data);

void pci_get_bar(pci_bar *bar, uint32_t id, uint32_t index);