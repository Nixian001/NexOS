#include "desc.h"
#include "../display/display.h"

void usb_print_device_desc(usb_device_desc *desc)
{
    print_string(" USB: Device Descriptor\n");
    print_nl();
    print_string("  USB Version: ");
    print_int(desc->usb_ver >> 8);
    print_nl();
    print_string("  USB Subversion: ");
    print_int((desc->usb_ver >> 4) & 0xf);
    print_nl();
    print_string("  Vendor ID: ");
    print_int(desc->vendor_id);
    print_nl();
    print_string("  Product ID: ");
    print_int(desc->product_id);
    print_nl();
    print_string("  Configurations: ");
    print_int(desc->conf_count);
    print_nl();
}

void usb_print_config_desc(usb_conf_desc *desc)
{
    print_string(" USB: Configuration Descriptor\n");
    print_nl();
    print_string("  Total Length: ");
    print_int(desc->total_len);
    print_nl();
    print_string("  Interfaces: ");
    print_int(desc->intf_count);
    print_nl();
    print_string("  Configuration Value: ");
    print_int(desc->conf_value);
    print_nl();
    print_string("  Configuration String: ");
    print_int(desc->conf_str);
    print_nl();
}

void usb_print_interface_desc(usb_intf_desc *desc)
{
    print_string(" USB: Interface Descriptor\n");
    print_nl();
    print_string("  Alternate Setting: ");
    print_int(desc->alt_setting);
    print_nl();
    print_string("  Endpoints: ");
    print_int(desc->endp_count);
    print_nl();
    print_string("  Class: ");
    print_int(desc->intf_class);
    print_nl();
    print_string("  Subclass: ");
    print_int(desc->intf_sub_class);
    print_nl();
    print_string("  Protocol: ");
    print_int(desc->intf_protocol);
    print_nl();
    print_string("  Interface String: ");
    print_int(desc->intf_str);
    print_nl();
}

void usb_print_endpoint_desc(usb_endp_desc *desc)
{
    print_string(" USB: Endpoint Descriptor\n");
    print_nl();
    print_string("  Address: ");
    print_int(desc->addr);
    print_nl();
    print_string("  Attributes: ");
    print_int(desc->attributes);
    print_nl();
    print_string("  Max Packet Size: ");
    print_int(desc->max_packet_size);
    print_nl();
    print_string("  Interval: ");
    print_int(desc->interval);
    print_nl();
}

void usb_print_hid_desc(usb_hid_desc *desc)
{
    print_string(" USB: HID Descriptor\n");
    print_nl();
    print_string("  HID Version: ");
    print_int(desc->hid_ver >> 8);
    print_nl();
    print_string("  Country Code: ");
    print_int(desc->country_code);
    print_nl();
    print_string("  Descriptor Count: ");
    print_int(desc->desc_count);
    print_nl();
    print_string("  Descriptor Type: ");
    print_int(desc->desc_type);
    print_nl();
    print_string("  Descriptor Length: ");
    print_int(desc->desc_len);
    print_nl();
}

void usb_print_hub_desc(usb_hub_desc *desc)
{
    print_string(" USB: Hub Descriptor\n");
    print_nl();
    print_string("  Port Count: ");
    print_int(desc->port_count);
    print_nl();
    print_string("  Characteristics: ");
    print_int(desc->chars);
    print_nl();
    print_string("  Port Power Time: ");
    print_int(desc->port_power_time);
    print_nl();
    print_string("  Current: ");
    print_int(desc->current);
    print_nl();
}