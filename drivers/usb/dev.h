#pragma once

#include "desc.h"
#include "req.h"

// USB Limits

#define USB_STRING_SIZE                 127

// USB Speeds

#define USB_FULL_SPEED                  0x00
#define USB_LOW_SPEED                   0x01
#define USB_HIGH_SPEED                  0x02

// USB Endpoint

typedef struct usb_endpoint
{
    usb_endp_desc desc;
    uint32_t toggle;
} usb_endpoint;

// USB Transfer

typedef struct usb_transfer
{
    usb_endpoint *endp;
    usb_dev_req *req;
    void *data;
    uint32_t len;
    bool complete;
    bool success;
} usb_transfer;

// USB Device

typedef struct usb_device
{
    struct usb_device *parent;
    struct usb_device *next;
    void *hc;
    void *drv;

    uint32_t port;
    uint32_t speed;
    uint32_t addr;
    uint32_t max_packet_size;

    usb_endpoint endp;

    usb_intf_desc intf_desc;

    void (*hc_control)(struct usb_device *dev, usb_transfer *t);
    void (*hc_intr)(struct usb_device *dev, usb_transfer *t);

    void (*drv_poll)(struct usb_device *dev);
} usb_device;

// Globals

extern usb_device *usb_device_list;

// Functions

usb_device *usb_dev_create();
bool usb_dev_init(usb_device *dev);
bool usb_dev_request(usb_device *dev,
    uint32_t type, uint32_t request,
    uint32_t value, uint32_t index,
    uint32_t len, void *data);
bool usb_dev_get_langs(usb_device *dev, uint16_t *langs);
bool usb_dev_get_string(usb_device *dev, char *str, uint32_t lang_id, uint32_t str_index);
bool usb_dev_clear_halt(usb_device *dev);