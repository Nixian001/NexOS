#include "ehci.h"

#include "controller.h"
#include "dev.h"

#include "../ports/ports.h"
#include "../../kernel/mem.h"
#include "../../kernel/util.h"
#include "../pci/pci_driver.h"
#include "../pci/pci_registry.h"
#include "../display/display.h"
#include "../../cpu/timer.h"
#include "../../cpu/link.h"

#define MAX_QH                          8
#define MAX_TD                          32

#define USBLEGSUP                       0x00        // USB Legacy Support Extended Capability
#define USBLEGCTLSTS                    0x04        // USB Legacy Support Control/Status

#define USBLEGSUP_HC_OS                 0x01000000  // HC OS Owned Semaphore
#define USBLEGSUP_HC_BIOS               0x00010000  // HC BIOS Owned Semaphore
#define USBLEGSUP_NCP_MASK              0x0000ff00  // Next EHCI Extended Capability Pointer
#define USBLEGSUP_CAPID                 0x000000ff  // Capability ID

typedef struct ehci_cap_regs {
    uint8_t cap_length;          // Capability Length
    uint8_t reserved;           // Reserved
    uint16_t hci_version;        // Host Controller Version
    uint32_t hcs_params;         // Host Controller Structural Parameters
    uint32_t hcc_params;         // Host Controller Capability Parameters
    uint32_t hcsp_port_route;    // Host Controller Structural Parameters Port Route
} ehci_cap_regs;

#define HCSPARAMS_N_PORTS_MASK          (15 << 0)   // Number of Ports
#define HCSPARAMS_PPC                   (1 << 4)    // Port Power Control
#define HCSPARAMS_PORT_ROUTE            (1 << 7)    // Port Routing Rules
#define HCSPARAMS_N_PCC_MASK            (15 << 8)   // Number of Ports per Companion Controller
#define HCSPARAMS_N_PCC_SHIFT           8
#define HCSPARAMS_N_CC_MASK             (15 << 12)  // Number of Companion Controllers
#define HCSPARAMS_N_CC_SHIFT            12
#define HCSPARAMS_P_INDICATOR           (1 << 16)   // Port Indicator
#define HCSPARAMS_DPN_MASK              (15 << 20)  // Debug Port Number
#define HCSPARAMS_DPN_SHIFT             20

#define HCCPARAMS_64_BIT                (1 << 0)    // 64-bit Addressing Capability
#define HCCPARAMS_PFLF                  (1 << 1)    // Programmable Frame List Flag
#define HCCPARAMS_ASPC                  (1 << 2)    // Asynchronous Schedule Park Capability
#define HCCPARAMS_IST_MASK              (15 << 4)   // Isochronous Scheduling Threshold
#define HCCPARAMS_EECP_MASK             (255 << 8)  // EHCI Extended Capabilities Pointer
#define HCCPARAMS_EECP_SHIFT            8

typedef struct ehci_op_regs {
    volatile uint32_t usb_cmd;              // USB Command
    volatile uint32_t usb_sts;              // USB Status
    volatile uint32_t usb_intr;             // USB Interrupt Enable
    volatile uint32_t frame_index;             // Frame Index
    volatile uint32_t ctrl_ds_segment;     // Control Data Segment Register
    volatile uint32_t periodic_list_base;  // Periodic Frame List Base Address
    volatile uint32_t async_list_addr;      // Asynchronous List Address
    volatile uint32_t reserved[9];         // Reserved
    volatile uint32_t config_flag;         // Configured Flag
    volatile uint32_t ports[];
} ehci_op_regs;


#define CMD_RS                          (1 << 0)    // Run/Stop
#define CMD_HCRESET                     (1 << 1)    // Host Controller Reset
#define CMD_FLS_MASK                    (3 << 2)    // Frame List Size
#define CMD_FLS_SHIFT                   2
#define CMD_PSE                         (1 << 4)    // Periodic Schedule Enable
#define CMD_ASE                         (1 << 5)    // Asynchronous Schedule Enable
#define CMD_IOAAD                       (1 << 6)    // Interrupt on Async Advance Doorbell
#define CMD_LHCR                        (1 << 7)    // Light Host Controller Reset
#define CMD_ASPMC_MASK                  (3 << 8)    // Asynchronous Schedule Park Mode Count
#define CMD_ASPMC_SHIFT                 8
#define CMD_ASPME                       (1 << 11)   // Asynchronous Schedule Park Mode Enable
#define CMD_ITC_MASK                    (255 << 16) // Interrupt Threshold Control
#define CMD_ITC_SHIFT                   16

#define STS_USBINT                      (1 << 0)    // USB Interrupt
#define STS_ERROR                       (1 << 1)    // USB Error Interrupt
#define STS_PCD                         (1 << 2)    // Port Change Detect
#define STS_FLR                         (1 << 3)    // Frame List Rollover
#define STS_HSE                         (1 << 4)    // Host System Error
#define STS_IOAA                        (1 << 5)    // Interrupt on Async Advance
#define STS_HCHALTED                    (1 << 12)   // Host Controller Halted
#define STS_RECLAMATION                 (1 << 13)   // Reclamation
#define STS_PSS                         (1 << 14)   // Periodic Schedule Status
#define STS_ASS                         (1 << 15)   // Asynchronous Schedule Status

#define INTR_USBINT                     (1 << 0)    // USB Interrupt Enable
#define INTR_ERROR                      (1 << 1)    // USB Error Interrupt Enable
#define INTR_PCD                        (1 << 2)    // Port Change Interrupt Enable
#define INTR_FLR                        (1 << 3)    // Frame List Rollover Enable
#define INTR_HSE                        (1 << 4)    // Host System Error Enable
#define INTR_IOAA                       (1 << 5)    // Interrupt on Async Advance Enable

#define FR_MASK                         0x3fff      // Frame Index Register

#define CF_PORT_ROUTE                   (1 << 0)    // Configure Flag (CF)

#define PORT_CONNECTION                 (1 << 0)    // Current Connect Status
#define PORT_CONNECTION_CHANGE          (1 << 1)    // Connect Status Change
#define PORT_ENABLE                     (1 << 2)    // Port Enabled
#define PORT_ENABLE_CHANGE              (1 << 3)    // Port Enable Change
#define PORT_OVER_CURRENT               (1 << 4)    // Over-current Active
#define PORT_OVER_CURRENT_CHANGE        (1 << 5)    // Over-current Change
#define PORT_FPR                        (1 << 6)    // Force Port Resume
#define PORT_SUSPEND                    (1 << 7)    // Suspend
#define PORT_RESET                      (1 << 8)    // Port Reset
#define PORT_LS_MASK                    (3 << 10)   // Line Status
#define PORT_LS_SHIFT                   10
#define PORT_POWER                      (1 << 12)   // Port Power
#define PORT_OWNER                      (1 << 13)   // Port Owner
#define PORT_IC_MASK                    (3 << 14)   // Port Indicator Control
#define PORT_IC_SHIFT                   14
#define PORT_TC_MASK                    (15 << 16)  // Port Test Control
#define PORT_TC_SHIFT                   16
#define PORT_WKCNNT_E                   (1 << 20)   // Wake on Connect Enable
#define PORT_WKDSCNNT_E                 (1 << 21)   // Wake on Disconnect Enable
#define PORT_WKOC_E                     (1 << 22)   // Wake on Over-current Enable
#define PORT_RWC                        (PORT_CONNECTION_CHANGE | PORT_ENABLE_CHANGE | PORT_OVER_CURRENT_CHANGE)

typedef struct ehci_td {
    volatile uint32_t link;
    volatile uint32_t alt_link;
    volatile uint32_t token;
    volatile uint32_t buffer[5];
    volatile uint32_t ext_buffer[5];

    // internal fields
    uint32_t td_next;
    uint32_t active;
    uint8_t  pad[4]; // padding for alignment
} ehci_td;

// TD Link Pointer
#define PTR_TERMINATE                   (1 << 0)

#define PTR_TYPE_MASK                   (3 << 1)
#define PTR_ITD                         (0 << 1)
#define PTR_QH                          (1 << 1)
#define PTR_SITD                        (2 << 1)
#define PTR_FSTN                        (3 << 1)

// TD Token
#define TD_TOK_PING                     (1 << 0)    // Ping State
#define TD_TOK_STS                      (1 << 1)    // Split Transaction State
#define TD_TOK_MMF                      (1 << 2)    // Missed Micro-Frame
#define TD_TOK_XACT                     (1 << 3)    // Transaction Error
#define TD_TOK_BABBLE                   (1 << 4)    // Babble Detected
#define TD_TOK_DATABUFFER               (1 << 5)    // Data Buffer Error
#define TD_TOK_HALTED                   (1 << 6)    // Halted
#define TD_TOK_ACTIVE                   (1 << 7)    // Active
#define TD_TOK_PID_MASK                 (3 << 8)    // PID Code
#define TD_TOK_PID_SHIFT                8
#define TD_TOK_CERR_MASK                (3 << 10)   // Error Counter
#define TD_TOK_CERR_SHIFT               10
#define TD_TOK_C_PAGE_MASK              (7 << 12)   // Current Page
#define TD_TOK_C_PAGE_SHIFT             12
#define TD_TOK_IOC                      (1 << 15)   // Interrupt on Complete
#define TD_TOK_LEN_MASK                 0x7fff0000  // Total Bytes to Transfer
#define TD_TOK_LEN_SHIFT                16
#define TD_TOK_D                        (1 << 31)   // Data Toggle
#define TD_TOK_D_SHIFT                  31

#define USB_PACKET_OUT                  0           // token 0xe1
#define USB_PACKET_IN                   1           // token 0x69
#define USB_PACKET_SETUP                2           // token 0x2d

typedef struct ehci_qh {
    uint32_t qhlp;      // Queue Head Horizontal Link Pointer
    uint32_t ch;        // Endpoint Characteristic
    uint32_t caps;      // Endpoint Capabilities
    volatile uint32_t cur_link; 

    volatile uint32_t next_link; // Next Link Pointer
    volatile uint32_t alt_link;  // Alternate Link Pointer
    volatile uint32_t token;     // Token
    volatile uint32_t buffer[5]; // Buffer Pointers
    volatile uint32_t ext_buffer[5]; // Extended Buffer Pointers

    usb_transfer *transfer;         // Pointer to the associated transfer
    link_t qh_link;                 // Link structure for managing the queue
    uint32_t td_head;
    uint32_t active;
    uint8_t  pad[20];
} ehci_qh;

// Endpoint Characteristics
#define QH_CH_DEVADDR_MASK              0x0000007f  // Device Address
#define QH_CH_INACTIVE                  0x00000080  // Inactive on Next Transaction
#define QH_CH_ENDP_MASK                 0x00000f00  // Endpoint Number
#define QH_CH_ENDP_SHIFT                8
#define QH_CH_EPS_MASK                  0x00003000  // Endpoint Speed
#define QH_CH_EPS_SHIFT                 12
#define QH_CH_DTC                       0x00004000  // Data Toggle Control
#define QH_CH_H                         0x00008000  // Head of Reclamation List Flag
#define QH_CH_MPL_MASK                  0x07ff0000  // Maximum Packet Length
#define QH_CH_MPL_SHIFT                 16
#define QH_CH_CONTROL                   0x08000000  // Control Endpoint Flag
#define QH_CH_NAK_RL_MASK               0xf0000000  // Nak Count Reload
#define QH_CH_NAK_RL_SHIFT              28

// Endpoint Capabilities
#define QH_CAP_INT_SCHED_SHIFT          0
#define QH_CAP_INT_SCHED_MASK           0x000000ff  // Interrupt Schedule Mask
#define QH_CAP_SPLIT_C_SHIFT            8
#define QH_CAP_SPLIT_C_MASK             0x0000ff00  // Split Completion Mask
#define QH_CAP_HUB_ADDR_SHIFT           16
#define QH_CAP_HUB_ADDR_MASK            0x007f0000  // Hub Address
#define QH_CAP_PORT_MASK                0x3f800000  // Port Number
#define QH_CAP_PORT_SHIFT               23
#define QH_CAP_MULT_MASK                0xc0000000  // High-Bandwidth Pipe Multiplier
#define QH_CAP_MULT_SHIFT               30

typedef struct ehci_controller {
    ehci_cap_regs *cap_regs;
    ehci_op_regs *op_regs;
    uint32_t *frame_list;
    ehci_qh *qh_pool;
    ehci_td *td_pool;
    ehci_qh *async_qh;
    ehci_qh *periodic_qh;
} ehci_controller;

#if 0

static void ehci_print_td(ehci_td *td) {
    print_string("TD: ");
    print_string("0x");
    print_int(td);
    print_nl();

    print_string("  > Link: 0x");
    print_int(td->link);
    print_nl();

    print_string("  > Alt Link: 0x");
    print_int(td->alt_link);
    print_nl();

    print_string("  > Token: 0x");
    print_int(td->token);
    print_nl();

    print_string("  > Buffer: 0x");
    print_int(td->buffer[0]);
    print_nl();
}

static void ehci_print_qh(ehci_qh *qh) {
    print_string("QH: ");
    print_string("0x");
    print_int(qh);
    print_nl();

    print_string("  > QHLP: 0x");
    print_int(qh->qhlp);
    print_nl();

    print_string("  > CH: 0x");
    print_int(qh->ch);
    print_nl();

    print_string("  > Caps: 0x");
    print_int(qh->caps);
    print_nl();

    print_string("  > Current Link: 0x");
    print_int(qh->cur_link);
    print_nl();

    print_string("  > Next Link: 0x");
    print_int(qh->next_link);
    print_nl();

    print_string("  > Alt Link: 0x");
    print_int(qh->alt_link);
    print_nl();

    print_string("  > Token: 0x");
    print_int(qh->token);
    print_nl();

    print_string("  > Buffer[0]: 0x");
    print_int(qh->buffer[0]);
    print_nl();

    print_string("  > QH PREV: 0x");
    print_int(qh->prev);
    print_nl();

    print_string("  > QH NEXT: 0x");
    print_int(qh->next);
    print_nl();
}

#endif

static ehci_td *ehci_alloc_td(ehci_controller *hc) {
    ehci_td *end = hc->td_pool + MAX_TD;
    for (ehci_td *td = hc->td_pool; td != end; td++) {
        if (td->active == 0) {
            td->active = 1;
            return td;
        }
    }

    print_string("ehci_alloc_td: No free TDs available.\n");
    return 0;
}

static ehci_qh *ehci_alloc_qh(ehci_controller *hc) {
    ehci_qh *end = hc->qh_pool + MAX_QH;
    for (ehci_qh *qh = hc->qh_pool; qh != end; qh++) {
        if (qh->active == 0) {
            qh->active = 1;
            return qh;
        }
    }

    print_string("ehci_alloc_qh: No free QHs available.\n");
    return 0;
}

static void ehci_free_td(ehci_td *td) {
    if (td) {
        td->active = 0;
    }
}

static void ehci_free_qh(ehci_qh *qh) {
    if (qh) {
        qh->active = 0;
    }
}

static void ehci_insert_async_qh(ehci_qh *list, ehci_qh *qh) {
    ehci_qh *end = link_data(list->qh_link.prev, ehci_qh, qh_link);

    qh->qhlp = PTR_TERMINATE;
    end->qhlp = (uint32_t)(uintptr_t)qh | PTR_QH;

    link_before(&list->qh_link, &qh->qh_link);
}

static void ehci_insert_periodic_qh(ehci_qh *list, ehci_qh *qh) {
    ehci_qh *end = link_data(list->qh_link.prev, ehci_qh, qh_link);

    qh->qhlp = PTR_TERMINATE;
    end->qhlp = (uint32_t)(uintptr_t)qh | PTR_QH;

    link_before(&list->qh_link, &qh->qh_link);
}

static void ehci_remove_qh(ehci_qh *qh)
{
    ehci_qh *prev = link_data(qh->qh_link.prev, ehci_qh, qh_link);

    prev->qhlp = qh->qhlp;
    link_remove(&qh->qh_link);
}

static void ehci_port_set(volatile uint32_t *port_reg, uint32_t data)
{
    uint32_t status = *port_reg;
    status |= data;
    status &= ~PORT_RWC;
    *port_reg = status;
}

static void ehci_port_clr(volatile uint32_t *port_reg, uint32_t data)
{
    uint32_t status = *port_reg;
    status &= ~PORT_RWC;
    status &= ~data;
    status |= PORT_RWC & data;
    *port_reg = status;
}

static void ehci_init_td(ehci_td *td, ehci_td *prev,
                          uint32_t toggle, uint32_t packet_type,
                          uint32_t len, const void *data)
{
    if (prev)
    {
        prev->link = (uint32_t)(uintptr_t)td;
        prev->td_next = (uint32_t)(uintptr_t)td;
    }

    td->link = PTR_TERMINATE;
    td->alt_link = PTR_TERMINATE;
    td->td_next = 0;

    td->token =
        (toggle << TD_TOK_D_SHIFT) |
        (len << TD_TOK_LEN_SHIFT) |
        (3 << TD_TOK_CERR_SHIFT) |
        (packet_type << TD_TOK_PID_SHIFT) |
        TD_TOK_ACTIVE;

    // Data buffer (not necessarily page aligned)
    uintptr_t p = (uintptr_t)data;
    td->buffer[0] = (uint32_t)p;
    td->ext_buffer[0] = (uint32_t)(p >> 32);
    p &= ~0xfff;

    // Remaining pages of buffer memory.
    for (uint32_t i = 1; i < 4; ++i)
    {
        p += 0x1000;
        td->buffer[i] = (uint32_t)(p);
        td->ext_buffer[i] = (uint32_t)(p >> 32);
    }
}

static void ehci_init_qh(ehci_qh *qh, usb_transfer *t, ehci_td *td, usb_device *parent, bool interrupt, uint32_t speed, uint32_t addr, uint32_t endp, uint32_t maxSize)
{
    qh->transfer = t;

    uint32_t ch =
        (maxSize << QH_CH_MPL_SHIFT) |
        QH_CH_DTC |
        (speed << QH_CH_EPS_SHIFT) |
        (endp << QH_CH_ENDP_SHIFT) |
        addr;
    uint32_t caps =
        (1 << QH_CAP_MULT_SHIFT);

    if (!interrupt)
    {
        ch |= 5 << QH_CH_NAK_RL_SHIFT;
    }

    if (speed != USB_HIGH_SPEED && parent)
    {
        if (interrupt)
        {
            // split completion mask - complete on frames 2, 3, or 4
            caps |= (0x1c << QH_CAP_SPLIT_C_SHIFT);
        }
        else
        {
            ch |= QH_CH_CONTROL;
        }

        caps |=
            (parent->port << QH_CAP_PORT_SHIFT) |
            (parent->addr << QH_CAP_HUB_ADDR_SHIFT);
    }

    if (interrupt)
    {
        // interrupt schedule mask - start on frame 0
        caps |= (0x01 << QH_CAP_INT_SCHED_SHIFT);
    }

    qh->ch = ch;
    qh->caps = caps;

    qh->td_head = (uint32_t)(uintptr_t)td;
    qh->next_link = (uint32_t)(uintptr_t)td;
    qh->token = 0;
}

static void ehci_process_qh(ehci_controller *hc, ehci_qh *qh)
{
    usb_transfer *t = qh->transfer;

    if (qh->token & TD_TOK_HALTED)
    {
        t->success = false;
        t->complete = true;
    }
    else if (qh->next_link & PTR_TERMINATE)
    {
        if (~qh->token & TD_TOK_ACTIVE)
        {
            if (qh->token & TD_TOK_DATABUFFER)
            {
                print_string(" Data Buffer Error\n");
            }
            if (qh->token & TD_TOK_BABBLE)
            {
                print_string(" Babble Detected\n");
            }
            if (qh->token & TD_TOK_XACT)
            {
                print_string(" Transaction Error\n");
            }
            if (qh->token & TD_TOK_MMF)
            {
                print_string(" Missed Micro-Frame\n");
            }

            t->success = true;
            t->complete = true;
        }
    }

    if (t->complete)
    {
        // Clear transfer from queue
        qh->transfer = 0;

        // Update endpoint toggle state
        if (t->success && t->endp)
        {
            t->endp->toggle ^= 1;
        }

        // Remove queue from schedule
        ehci_remove_qh(qh);

        // Free transfer descriptors
        ehci_td *td = (ehci_td *)(uintptr_t)qh->td_head;
        while (td)
        {
            ehci_td *next = (ehci_td *)(uintptr_t)td->td_next;
            ehci_free_td(td);
            td = next;
        }

        // Free queue head
        ehci_free_qh(qh);
    }
}

static void ehci_wait_for_qh(ehci_controller *hc, ehci_qh *qh)
{
    usb_transfer *t = qh->transfer;

    while (!t->complete)
    {
        ehci_process_qh(hc, qh);
    }
}

static uint32_t ehci_reset_port(ehci_controller *hc, uint32_t port)
{
    volatile uint32_t *reg = &hc->op_regs->ports[port];

    // Reset the port
    ehci_port_set(reg, PORT_RESET);
    timer_sleep(50);
    ehci_port_clr(reg, PORT_RESET);

    // Wait 100ms for port to enable (TODO - what is appropriate length of time?)
    uint32_t status = 0;
    for (uint32_t i = 0; i < 10; ++i)
    {
        // Delay
        timer_sleep(10);

        // Get current status
        status = *reg;

        // Check if device is attached to port
        if (~status & PORT_CONNECTION)
        {
            break;
        }

        // Acknowledge change in status
        if (status & (PORT_ENABLE_CHANGE | PORT_CONNECTION_CHANGE))
        {
            ehci_port_clr(reg, PORT_ENABLE_CHANGE | PORT_CONNECTION_CHANGE);
            continue;
        }

        // Check if device is enabled
        if (status & PORT_ENABLE)
        {
            break;
        }
    }

    return status;
}

static void ehci_dev_control(usb_device *dev, usb_transfer *t)
{
    ehci_controller *hc = (ehci_controller *)dev->hc;
    usb_dev_req *req = t->req;

    // Determine transfer properties
    uint32_t speed = dev->speed;
    uint32_t addr = dev->addr;
    uint32_t maxSize = dev->max_packet_size;
    uint32_t type = req->type;
    uint32_t len = req->len;

    // Create queue of transfer descriptors
    ehci_td *td = ehci_alloc_td(hc);
    if (!td)
    {
        return;
    }

    ehci_td *head = td;
    ehci_td *prev = 0;

    // Setup packet
    uint32_t toggle = 0;
    uint32_t packet_type = USB_PACKET_SETUP;
    uint32_t packet_size = sizeof(usb_dev_req);
    ehci_init_td(td, prev, toggle, packet_type, packet_size, req);
    prev = td;

    // Data in/out packets
    packet_type = type & RT_DEV_TO_HOST ? USB_PACKET_IN : USB_PACKET_OUT;

    uint8_t *it = (uint8_t *)t->data;
    uint8_t *end = it + len;
    while (it < end)
    {
        td = ehci_alloc_td(hc);
        if (!td)
        {
            return;
        }

        toggle ^= 1;
        packet_size = end - it;
        if (packet_size > maxSize)
        {
            packet_size = maxSize;
        }

        ehci_init_td(td, prev, toggle, packet_type, packet_size, it);

        it += packet_size;
        prev = td;
    }

    // Status packet
    td = ehci_alloc_td(hc);
    if (!td)
    {
        return;
    }

    toggle = 1;
    packet_type = type & RT_DEV_TO_HOST ? USB_PACKET_OUT : USB_PACKET_IN;
    ehci_init_td(td, prev, toggle, packet_type, 0, 0);

    // Initialize queue head
    ehci_qh *qh = ehci_alloc_qh(hc);
    ehci_init_qh(qh, t, head, dev->parent, false, speed, addr, 0, maxSize);

    // Wait until queue has been processed
    ehci_insert_async_qh(hc->async_qh, qh);
    ehci_wait_for_qh(hc, qh);
}

static void ehci_dev_intr(usb_device *dev, usb_transfer *t)
{
    ehci_controller *hc = (ehci_controller *)dev->hc;

    // Determine transfer properties
    uint32_t speed = dev->speed;
    uint32_t addr = dev->addr;
    uint32_t max_size = dev->max_packet_size;
    uint32_t endp = dev->endp.desc.addr & 0xf;

    // Create queue of transfer descriptors
    ehci_td *td = ehci_alloc_td(hc);
    if (!td)
    {
        t->success = false;
        t->complete = true;
        return;
    }

    ehci_td *head = td;
    ehci_td *prev = 0;

    // Data in/out packets
    uint32_t toggle = dev->endp.toggle;
    uint32_t packet_type = USB_PACKET_IN;
    uint32_t packet_size = t->len;

    ehci_init_td(td, prev, toggle, packet_type, packet_size, t->data);

    // Initialize queue head
    ehci_qh *qh = ehci_alloc_qh(hc);
    ehci_init_qh(qh, t, head, dev->parent, true, speed, addr, endp, max_size);

    // Schedule queue
    ehci_insert_periodic_qh(hc->periodic_qh, qh);
}

static void ehci_probe(ehci_controller *hc)
{
    // Port setup
    uint32_t port_count = hc->cap_regs->hcs_params & HCSPARAMS_N_PORTS_MASK;
    for (uint32_t port = 0; port < port_count; ++port)
    {
        // Reset port
        uint32_t status = ehci_reset_port(hc, port);

        if (status & PORT_ENABLE)
        {
            uint32_t speed = USB_HIGH_SPEED;

            usb_device *dev = usb_dev_create();
            if (dev)
            {
                dev->parent = 0;
                dev->hc = hc;
                dev->port = port;
                dev->speed = speed;
                dev->max_packet_size = 8;

                dev->hc_control = ehci_dev_control;
                dev->hc_intr = ehci_dev_intr;

                if (!usb_dev_init(dev))
                {
                    // TODO - cleanup
                }
            }
        }
    }
}

#define link_data(link,T,m) (T *)((char *)(link) - (unsigned long)(&(((T*)0)->m)))

static void ehci_controller_poll_list(ehci_controller *hc, link_t *list)
{
    ehci_qh *qh;
    ehci_qh *next;
    for (qh = link_data(list->next, ehci_qh, qh_link), 
         next = link_data(qh->qh_link.next, ehci_qh, qh_link);
         &qh->qh_link != list;
         qh = next, 
         next = link_data(next->qh_link.next, ehci_qh, qh_link)) {
        if (qh->transfer)
        {
            ehci_process_qh(hc, qh);
        }
    }
}

static void ehci_controller_poll(usb_controller *controller)
{
    ehci_controller *hc = (ehci_controller *)controller->hc;

    ehci_controller_poll_list(hc, &hc->async_qh->qh_link);
    ehci_controller_poll_list(hc, &hc->periodic_qh->qh_link);
}

void ehci_init(uint32_t id, const pci_device_info *info)
{
    if (!(((info->class_code << 8) | info->subclass) == PCI_SERIAL_USB &&
        info->prog_intf == PCI_SERIAL_USB_EHCI))
    {
        return;
    }

    if (sizeof(ehci_qh) != 128)
    {
        // ConsolePrint("Unexpected ehci_qh size: %d\n", sizeof(ehci_qh));
        return;
    }

    // ConsolePrint ("Initializing EHCI\n");

    // Base I/O Address
    pci_bar bar;
    pci_get_bar(&bar, id, 0);
    if (bar.flags & PCI_BAR_IO)
    {
        // Only Memory Mapped I/O supported
        return;
    }

    // Controller initialization
    ehci_controller *hc = mem_alloc(sizeof(ehci_controller));
    hc->cap_regs = (ehci_cap_regs *)(uintptr_t)bar.u.address;
    hc->op_regs = (ehci_op_regs *)(uintptr_t)(bar.u.address + hc->cap_regs->cap_length);
    hc->frame_list = (uint32_t *)mem_alloc(1024 * sizeof(uint32_t));
    hc->qh_pool = (ehci_qh *)mem_alloc(sizeof(ehci_qh) * MAX_QH);
    hc->td_pool = (ehci_td *)mem_alloc(sizeof(ehci_td) * MAX_TD);

    // Asynchronous queue setup
    ehci_qh *qh = ehci_alloc_qh(hc);
    qh->qhlp = (uint32_t)(uintptr_t)qh | PTR_QH;
    qh->ch = QH_CH_H;
    qh->caps = 0;
    qh->cur_link = 0;
    qh->next_link = PTR_TERMINATE;
    qh->alt_link = 0;
    qh->token = 0;
    for (uint8_t i = 0; i < 5; ++i)
    {
        qh->buffer[i] = 0;
        qh->ext_buffer[i] = 0;
    }
    qh->transfer = 0;
    qh->qh_link.prev = &qh->qh_link;
    qh->qh_link.next = &qh->qh_link;

    hc->async_qh = qh;

    // Periodic list queue setup
    qh = ehci_alloc_qh(hc);
    qh->qhlp = PTR_TERMINATE;
    qh->ch = 0;
    qh->caps = 0;
    qh->cur_link = 0;
    qh->next_link = PTR_TERMINATE;
    qh->alt_link = 0;
    qh->token = 0;
    for (uint8_t i = 0; i < 5; ++i)
    {
        qh->buffer[i] = 0;
        qh->ext_buffer[i] = 0;
    }
    qh->transfer = 0;
    qh->qh_link.prev = &qh->qh_link;
    qh->qh_link.next = &qh->qh_link;

    hc->periodic_qh = qh;
    for (uint8_t i = 0; i < 1024; ++i)
    {
        hc->frame_list[i] = PTR_QH | (uint32_t)(uintptr_t)qh;
    }

    // Check extended capabilities
    uint16_t eecp = (hc->cap_regs->hcc_params & HCCPARAMS_EECP_MASK) >> HCCPARAMS_EECP_SHIFT;
    if (eecp >= 0x40)
    {
        // Disable BIOS legacy support
        uint32_t legsup = pci_read_32(id, eecp + USBLEGSUP);

        if (legsup & USBLEGSUP_HC_BIOS)
        {
            pci_write_32(id, eecp + USBLEGSUP, legsup | USBLEGSUP_HC_OS);
            for (;;)
            {
                legsup = pci_read_32(id, eecp + USBLEGSUP);
                if (~legsup & USBLEGSUP_HC_BIOS && legsup & USBLEGSUP_HC_OS)
                {
                    break;
                }
            }
        }
    }

    // Disable interrupts
    hc->op_regs->usb_intr = 0;

    // Setup frame list
    hc->op_regs->frame_index = 0;
    hc->op_regs->periodic_list_base = (uint32_t)(uintptr_t)hc->frame_list;
    hc->op_regs->async_list_addr = (uint32_t)(uintptr_t)hc->async_qh;
    hc->op_regs->ctrl_ds_segment = 0;

    // Clear status
    hc->op_regs->usb_sts = 0x3f;

    // Enable controller
    hc->op_regs->usb_cmd = (8 << CMD_ITC_SHIFT) | CMD_PSE | CMD_ASE | CMD_RS;
    while (hc->op_regs->usb_sts & STS_HCHALTED) // TODO - remove after dynamic port detection
        ;

    // Configure all devices to be managed by the EHCI
    hc->op_regs->config_flag = 1;
    timer_sleep(5);    // TODO - remove after dynamic port detection

    // Probe devices
    ehci_probe(hc);

    // Register controller
    usb_controller *controller = (usb_controller *)(mem_alloc(sizeof(usb_controller)));
    controller->next = usb_controller_list;
    controller->hc = hc;
    controller->poll = ehci_controller_poll;

    usb_controller_list = controller;
}