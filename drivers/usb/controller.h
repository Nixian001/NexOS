#pragma once

typedef struct usb_controller {
    struct usb_controller *next;
    void *hc;

    void (*poll)(struct usb_controller *controller);
} usb_controller;

extern usb_controller *usb_controller_list;