#pragma once

#include <stdint.h>

// USB Base Descriptor Types

#define USB_DESC_DEVICE                 0x01
#define USB_DESC_CONF                   0x02
#define USB_DESC_STRING                 0x03
#define USB_DESC_INTF                   0x04
#define USB_DESC_ENDP                   0x05

// USB HID Descriptor Types

#define USB_DESC_HID                    0x21
#define USB_DESC_REPORT                 0x22
#define USB_DESC_PHYSICAL               0x23

// USB HUB Descriptor Types

#define USB_DESC_HUB                    0x29

// USB Device Descriptor

typedef struct usb_device_desc {
    uint8_t len;
    uint8_t type;
    uint16_t usb_ver;
    uint8_t dev_class;
    uint8_t dev_sub_class;
    uint8_t dev_protocol;
    uint8_t max_packet_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_ver;
    uint8_t vendor_str;
    uint8_t product_str;
    uint8_t serial_str;
    uint8_t conf_count;
} usb_device_desc;

// USB Configuration Descriptor

typedef struct usb_conf_desc
{
    uint8_t len;
    uint8_t type;
    uint16_t total_len;
    uint8_t intf_count;
    uint8_t conf_value;
    uint8_t conf_str;
    uint8_t attributes;
    uint8_t max_power;
} usb_conf_desc;

// USB String Descriptor

typedef struct usb_string_desc
{
    uint8_t len;
    uint8_t type;
    uint16_t str[];
} usb_string_desc;

typedef struct usb_intf_desc
{
    uint8_t len;
    uint8_t type;
    uint8_t intf_index;
    uint8_t alt_setting;
    uint8_t endp_count;
    uint8_t intf_class;
    uint8_t intf_sub_class;
    uint8_t intf_protocol;
    uint8_t intf_str;
} usb_intf_desc;

// USB Endpoint Descriptor

typedef struct usb_endp_desc
{
    uint8_t len;
    uint8_t type;
    uint8_t addr;
    uint8_t attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} usb_endp_desc;

// USB HID Descriptor

typedef struct usb_hid_desc
{
    uint8_t len;
    uint8_t type;
    uint16_t hid_ver;
    uint8_t country_code;
    uint8_t desc_count;
    uint8_t desc_type;
    uint16_t desc_len;
} usb_hid_desc;

// USB Hub Descriptor

typedef struct usb_hub_desc
{
    uint8_t len;
    uint8_t type;
    uint8_t port_count;
    uint16_t chars;
    uint8_t port_power_time;
    uint8_t current;
    // removable/power control bits vary in size
} usb_hub_desc;

// Hub Characteristics
#define HUB_POWER_MASK                  0x03        // Logical Power Switching Mode
#define HUB_POWER_GLOBAL                0x00
#define HUB_POWER_INDIVIDUAL            0x01
#define HUB_COMPOUND                    0x04        // Part of a Compound Device
#define HUB_CURRENT_MASK                0x18        // Over-current Protection Mode
#define HUB_TT_TTI_MASK                 0x60        // TT Think Time
#define HUB_PORT_INDICATORS             0x80        // Port Indicators

// Functions

void usb_print_device_desc(usb_device_desc *desc);
void usb_print_config_desc(usb_conf_desc *desc);
void usb_print_interface_desc(usb_intf_desc *desc);
void usb_print_endpoint_desc(usb_endp_desc *desc);

void usb_print_hid_desc(usb_hid_desc *desc);
void usb_print_hub_desc(usb_hub_desc *desc);