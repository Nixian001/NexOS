#pragma once

#include "../pci/pci_driver.h"

void ehci_init(uint32_t id, const pci_device_info *info);