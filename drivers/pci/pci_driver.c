#include "pci_driver.h"

#include "../../kernel/util.h"
#include "../ports/ports.h"
#include "../usb/ehci.h"

const pci_driver pci_driver_table[] = {
    // Add PCI drivers here
    { ehci_init },
    { 0 },
};

// Function to read a byte from a PCI configuration space
// id: Device ID, reg: Register offset
uint8_t pci_read_8(uint32_t id, uint32_t reg) {
    uint32_t address = 0x80000000 | id | (reg & 0xFC);
    port_word_out(PCI_CONFIG_ADDR, address);
    return port_byte_in(PCI_CONFIG_DATA + (reg & 0x03));
}

uint16_t pci_read_16(uint32_t id, uint32_t reg) {
    uint32_t address = 0x80000000 | id | (reg & 0xFC);
    port_word_out(PCI_CONFIG_ADDR, address);
    return port_byte_in(PCI_CONFIG_DATA + (reg & 0x02));
}

uint32_t pci_read_32(uint32_t id, uint32_t reg) {
    uint32_t address = 0x80000000 | id | (reg & 0xFC);
    port_word_out(PCI_CONFIG_ADDR, address);
    return port_byte_in(PCI_CONFIG_DATA + (reg & 0x00));
}

void pci_write_8(uint32_t id, uint32_t reg, uint8_t data) {
    uint32_t address = 0x80000000 | id | (reg & 0xFC);
    port_word_out(PCI_CONFIG_ADDR, address);
    port_byte_out(PCI_CONFIG_DATA + (reg & 0x03), data);
}

void pci_write_16(uint32_t id, uint32_t reg, uint16_t data) {
    uint32_t address = 0x80000000 | id | (reg & 0xFC);
    port_word_out(PCI_CONFIG_ADDR, address);
    port_byte_out(PCI_CONFIG_DATA + (reg & 0x02), data);
}

void pci_write_32(uint32_t id, uint32_t reg, uint32_t data) {
    uint32_t address = 0x80000000 | id | (reg & 0xFC);
    port_word_out(PCI_CONFIG_ADDR, address);
    port_byte_out(PCI_CONFIG_DATA + (reg & 0x00), data);
}

static void pci_read_bar(uint32_t id, uint32_t index, uint32_t *address, uint32_t *mask) {
    uint32_t reg = PCI_CONFIG_BAR0 + index * sizeof(uint32_t);

    *address = pci_read_32(id, reg);

    pci_write_32(id, reg, 0xFFFFFFFF); // Set to all ones to read the mask
    *mask = pci_read_32(id, reg);

    pci_write_32(id, reg, *address); // Restore the original address
}

void pci_get_bar(pci_bar *bar, uint32_t id, uint32_t index) {
    uint32_t address_low, mask_low;
    pci_read_bar(id, index, &address_low, &mask_low);

    if (address_low & PCI_BAR_64) {
        // 64-bit MMIO
        uint32_t address_high, mask_high;
        pci_read_bar(id, index + 1, &address_high, &mask_high);

        bar->u.address = (void *)(((uint64_t)address_high << 32) | (address_low & ~0xF));
        bar->size = (((uint64_t)mask_high << 32) | (mask_low & ~0xF)) + 1;
        bar->flags = address_low & 0xF; // Store flags
    } else if (address_low & PCI_BAR_IO) {
        // I/O Register
        bar->u.port = address_low & ~0x3; // Clear the lower bits
        bar->size = ~(mask_low & ~0x3) + 1; // Calculate size
        bar->flags = address_low & 0x3; // Store flags
    } else {
        // 32-bit MMIO
        bar->u.address = (void *)(uintptr_t)(address_low & ~0xF); // Clear
        bar->size = ~(mask_low & ~0xF) + 1; // Calculate size
        bar->flags = address_low & 0xF; // Store flags
    }
}