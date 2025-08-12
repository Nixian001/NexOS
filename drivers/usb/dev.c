#include "dev.h"
#include "driver.h"
#include "../display/display.h"
#include "../../kernel/mem.h"
#include "../../cpu/timer.h"

#include <stdbool.h>

usb_device *usb_device_list;

static int next_usb_addr;

usb_device *usb_dev_create()
{
    // Initialize structure
    usb_device *dev = mem_alloc(sizeof(usb_device));
    if (dev)
    {
        dev->parent = 0;
        dev->next = usb_device_list;
        dev->hc = 0;
        dev->drv = 0;

        dev->port = 0;
        dev->speed = 0;
        dev->addr = 0;
        dev->max_packet_size = 0;
        dev->endp.toggle = 0;

        dev->hc_control = 0;
        dev->hc_intr = 0;
        dev->drv_poll = 0;

        usb_device_list = dev;
    }

    return dev;
}

bool usb_dev_init(usb_device *dev)
{
    // Get first 8 bytes of device descriptor
    usb_device_desc dev_desc;
    if (!usb_dev_request(dev,
        RT_DEV_TO_HOST | RT_STANDARD | RT_DEV,
        REQ_GET_DESC, (USB_DESC_DEVICE << 8) | 0, 0,
        8, &dev_desc))
    {
        return false;
    }

    dev->max_packet_size = dev_desc.max_packet_size;

    // Set address
    uint32_t addr = ++next_usb_addr;

    if (!usb_dev_request(dev,
        RT_HOST_TO_DEV | RT_STANDARD | RT_DEV,
        REQ_SET_ADDR, addr, 0,
        0, 0))
    {
        return false;
    }

    dev->addr = addr;

    timer_sleep(2);    // Set address recovery time

    // Read entire descriptor
    if (!usb_dev_request(dev,
        RT_DEV_TO_HOST | RT_STANDARD | RT_DEV,
        REQ_GET_DESC, (USB_DESC_DEVICE << 8) | 0, 0,
        sizeof(usb_device_desc), &dev_desc))
    {
        return false;
    }

    // Dump descriptor
    usb_print_device_desc(&dev_desc);

    // String Info
    uint16_t langs[USB_STRING_SIZE];
    usb_dev_get_langs(dev, langs);

    uint16_t lang_id = langs[0];
    if (lang_id)
    {
        char product_str[USB_STRING_SIZE];
        char vendor_str[USB_STRING_SIZE];
        char serial_str[USB_STRING_SIZE];
        usb_dev_get_string(dev, product_str, lang_id, dev_desc.product_str);
        usb_dev_get_string(dev, vendor_str, lang_id, dev_desc.vendor_str);
        usb_dev_get_string(dev, serial_str, lang_id, dev_desc.serial_str);

        
        // ConsolePrint("  Product='%s' Vendor='%s' Serial=%s\n", product_str, vendor_str, serial_str);
        print_string("  Product='");
        print_string(product_str);
        print_string("' Vendor='");
        print_string(vendor_str);
        print_string("' Serial='");
        print_string(serial_str);
        print_nl();
    }

    // Pick configuration and interface - grab first for now
    uint8_t config_buf[256];
    uint16_t picked_conf_value = 0;
    usb_intf_desc *picked_intf_desc = 0;
    usb_endp_desc *picked_endp_desc = 0;

    for (uint16_t conf_index = 0; conf_index < dev_desc.conf_count; ++conf_index)
    {
        // Get configuration total length
        if (!usb_dev_request(dev,
            RT_DEV_TO_HOST | RT_STANDARD | RT_DEV,
            REQ_GET_DESC, (USB_DESC_CONF << 8) | conf_index, 0,
            4, config_buf))
        {
            continue;
        }

        // Only static size supported for now
        usb_conf_desc *conf_desc = (usb_conf_desc *)config_buf;
        if (conf_desc->total_len > sizeof(config_buf))
        {
            print_string("  Configuration length '");
            print_int(conf_desc->total_len);
            print_string("' greater than '");
            print_int(sizeof(config_buf));
            print_string("' bytes\n");
            continue;
        }

        // Read all configuration data
        if (!usb_dev_request(dev,
            RT_DEV_TO_HOST | RT_STANDARD | RT_DEV,
            REQ_GET_DESC, (USB_DESC_CONF << 8) | conf_index, 0,
            conf_desc->total_len, config_buf))
        {
            continue;
        }

        usb_print_config_desc(conf_desc);

        if (!picked_conf_value)
        {
            picked_conf_value = conf_desc->conf_value;
        }

        // Parse configuration data
        uint8_t *data = config_buf + conf_desc->len;
        uint8_t *end = config_buf + conf_desc->total_len;

        while (data < end)
        {
            uint8_t len = data[0];
            uint8_t type = data[1];

            switch (type)
            {
            case USB_DESC_INTF:
                {
                    usb_intf_desc *intf_desc = (usb_intf_desc *)data;
                    usb_print_interface_desc(intf_desc);

                    if (!picked_intf_desc)
                    {
                        picked_intf_desc = intf_desc;
                    }
                }
                break;

            case USB_DESC_ENDP:
                {
                    usb_endp_desc *endp_desc = (usb_endp_desc *)data;
                    usb_print_endpoint_desc(endp_desc);

                    if (!picked_endp_desc)
                    {
                        picked_endp_desc = endp_desc;
                    }
                }
                break;
            }

            data += len;
        }
    }

    // Configure device
    if (picked_conf_value && picked_intf_desc && picked_endp_desc)
    {
        if (!usb_dev_request(dev,
            RT_HOST_TO_DEV | RT_STANDARD | RT_DEV,
            REQ_SET_CONF, picked_conf_value, 0,
            0, 0))
        {
            return false;
        }

        dev->intf_desc = *picked_intf_desc;
        dev->endp.desc = *picked_endp_desc;

        // Initialize driver
        const usb_driver *driver = usb_driver_table;
        while (driver->init)
        {
            if (driver->init(dev))
            {
                break;
            }

            ++driver;
        }
    }

    return true;
}

bool usb_dev_request(usb_device *dev,
    uint32_t type, uint32_t request,
    uint32_t value, uint32_t index,
    uint32_t len, void *data)
{
    usb_dev_req req;
    req.type = type;
    req.req = request;
    req.value = value;
    req.index = index;
    req.len = len;

    usb_transfer t;
    t.endp = 0;
    t.req = &req;
    t.data = data;
    t.len = len;
    t.complete = false;
    t.success = false;

    dev->hc_control(dev, &t);

    return (bool)t.success;
}

bool usb_dev_get_langs(usb_device *dev, uint16_t *langs)
{
    langs[0] = 0;

    uint8_t buf[256];
    usb_string_desc *desc = (usb_string_desc *)buf;

    // Get length
    if (!usb_dev_request(dev,
        RT_DEV_TO_HOST | RT_STANDARD | RT_DEV,
        REQ_GET_DESC, (USB_DESC_STRING << 8) | 0, 0,
        1, desc))
    {
        return false;
    }

    // Get lang data
    if (!usb_dev_request(dev,
        RT_DEV_TO_HOST | RT_STANDARD | RT_DEV,
        REQ_GET_DESC, (USB_DESC_STRING << 8) | 0, 0,
        desc->len, desc))
    {
        return false;
    }

    uint32_t lang_len = (desc->len - 2) / 2;
    for (uint32_t i = 0; i < lang_len; ++i)
    {
        langs[i] = desc->str[i];
    }

    langs[lang_len] = 0;
    return true;
}

bool usb_dev_get_string(usb_device *dev, char *str, uint32_t lang_id, uint32_t str_index)
{
    str[0] = '\0';
    if (!str_index)
    {
        return true;
    }

    uint8_t buf[256];
    usb_string_desc *desc = (usb_string_desc *)buf;

    // Get string length
    if (!usb_dev_request(dev,
        RT_DEV_TO_HOST | RT_STANDARD | RT_DEV,
        REQ_GET_DESC, (USB_DESC_STRING << 8) | str_index, lang_id,
        1, desc))
    {
        return false;
    }

    // Get string data
    if (!usb_dev_request(dev,
        RT_DEV_TO_HOST | RT_STANDARD | RT_DEV,
        REQ_GET_DESC, (USB_DESC_STRING << 8) | str_index, lang_id,
        desc->len, desc))
    {
        return false;
    }

    // Dumb Unicode to ASCII conversion
    uint32_t str_len = (desc->len - 2) / 2;
    for (uint32_t i = 0; i < str_len; ++i)
    {
        str[i] = desc->str[i];
    }

    str[str_len] = '\0';
    return true;
}

bool usb_dev_clear_halt(usb_device *dev)
{
    return usb_dev_request(dev,
        RT_DEV_TO_HOST | RT_STANDARD | RT_ENDP,
        REQ_CLEAR_FEATURE,
        F_ENDPOINT_HALT,
        dev->endp.desc.addr & 0xf,
        0, 0);
}