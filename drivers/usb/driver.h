#pragma once

#include "dev.h"

typedef struct usb_driver {
    bool (*init)(usb_device *dev);
} usb_driver;

extern const usb_driver usb_driver_table[];