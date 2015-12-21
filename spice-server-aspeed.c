/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
   Copyright (C) 2009-2015 Red Hat, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Test ground for developing specific tests.
 *
 * Any specific test can start of from here and set the server to the
 * specific required state, and create specific operations or reuse
 * existing ones in the test_display_base supplied queue.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <common/mem.h>
#include <glib.h>

#include "spice-server-aspeed.h"

#define ASPEED_ENCODER_VIDEOCAP_DEV	"/dev/videocap"
#define ASPEED_USB_DEV			"/dev/usb"

#define SCANCODE_GREY	0x80
#define SCANCODE_EMUL0	0xE0
#define SCANCODE_UP	0x80

SpiceCoreInterface *core;
#if 1
static SpiceTimer *ping_timer;

static int ping_ms = 100;

static void pinger(SPICE_GNUC_UNUSED void *opaque)
{
    // show_channels is not thread safe - fails if disconnections / connections occur
    //show_channels(server);

    core->timer_start(ping_timer, ping_ms);
}
#endif

static int iusb_request(iUSBSpice *iusb, int devtype)
{
    IUSB_FREE_DEVICE_INFO devinfo;
    IUSB_REQ_REL_DEVICE_INFO relinfo;

    bzero(&devinfo, sizeof(IUSB_FREE_DEVICE_INFO));
    devinfo.DeviceType = devtype;
    devinfo.LockType = LOCK_TYPE_EXCLUSIVE;

    ioctl(iusb->fd, USB_GET_INTERFACES, &devinfo);

    if (!devinfo.iUSBdevList.DevCount) {
        return FALSE;
    }

    bzero(&relinfo, sizeof(IUSB_REQ_REL_DEVICE_INFO));
    relinfo.DevInfo.DeviceType = devtype;
    relinfo.DevInfo.DevNo = devinfo.iUSBdevList.DevInfo.DevNo;
    relinfo.DevInfo.IfNum = devinfo.iUSBdevList.DevInfo.IfNum;
    relinfo.DevInfo.LockType = LOCK_TYPE_EXCLUSIVE;

    printf("relinfo devtype=%d devno=%d ifnum=%d\n", devtype, devinfo.iUSBdevList.DevInfo.DevNo, devinfo.iUSBdevList.DevInfo.IfNum);

    if (ioctl(iusb->fd, USB_REQ_INTERFACE, &relinfo) < 0)
        return FALSE;

    iusb->key  = relinfo.Key;
    iusb->addr = relinfo.DevInfo.DevNo << 8 | relinfo.DevInfo.IfNum;

    return TRUE;
}

static int iusb_release(iUSBSpice *iusb, int devtype)
{
    IUSB_REQ_REL_DEVICE_INFO relinfo;

    bzero(&relinfo, sizeof(IUSB_REQ_REL_DEVICE_INFO));

    relinfo.Key = iusb->key;
    relinfo.DevInfo.DeviceType = devtype;
    relinfo.DevInfo.DevNo = iusb->addr >> 8;
    relinfo.DevInfo.IfNum = iusb->addr & 0xff;
    relinfo.DevInfo.LockType = LOCK_TYPE_EXCLUSIVE;

    return TRUE;
}

static void iusb_set_mouse_mode(iUSBSpice *iusb, int mode)
{
    uint32_t dir;
    IUSB_IOCTL_DATA ioc;

    ioc.Key = iusb->key;
    ioc.Data = 0;
    ioc.DevInfo.DeviceType = IUSB_DEVICE_MOUSE;
    ioc.DevInfo.DevNo = iusb->addr >> 8;
    ioc.DevInfo.IfNum = iusb->addr & 0xff;

    printf("mouse get ioctl=%d ", ioctl(iusb->fd, MOUSE_GET_CURRENT_MODE, &ioc));
    printf(" mode=%d\n", ioc.Data);

    if (mode == 1 && ioc.Data != 1) {
         ioc.Data = 1;
         dir = MOUSE_ABS_TO_REL;
    } else if (mode == 0 && ioc.Data != 2) {
         ioc.Data = 2;
         dir = MOUSE_REL_TO_ABS;
    } else {
         return;
    }

    printf("mouse set ioctl=%d ", ioctl(iusb->fd, dir, &ioc));
    printf(" mode=%d\n", ioc.Data);
}

static void kbd_push_key(SpiceKbdInstance *sin, uint8_t scancode)
{
    iUSBSpiceKbd *kbd = (iUSBSpiceKbd *)((void *)sin - sizeof(iUSBSpice));
    int keycode;
    int up;
    int i;
    int j = 0;
    uint8_t *p;
    IUSB_HID_PACKET *hid;
    uint8_t *pkt;

    if (scancode == SCANCODE_EMUL0) {
        kbd->emul0 = TRUE;
        return;
    }
    keycode = scancode & ~SCANCODE_UP;
    up = scancode & SCANCODE_UP;
    if (kbd->emul0) {
        kbd->emul0 = FALSE;
        keycode |= SCANCODE_GREY;
    }

    pkt = calloc(sizeof(IUSB_HID_PACKET) + 8, 1);
    hid = (void *)pkt;
    memcpy(hid->Header.Signature, IUSB_SIG, sizeof(IUSB_SIG));
    hid->Header.Major = IUSB_MAJOR;
    hid->Header.Minor = IUSB_MINOR;
    hid->Header.HeaderLen = sizeof(IUSB_HEADER);
    hid->Header.HeaderCheckSum = 0;
    hid->Header.DataPktLen = 9;
    hid->Header.ServerCaps = 0;
    hid->Header.DeviceType = IUSB_DEVICE_KEYBD;
    hid->Header.Protocol = IUSB_PROTO_KEYBD_DATA;
    hid->Header.Direction = FROM_REMOTE;
    hid->Header.DeviceNo = kbd->iusb.addr >> 8;
    hid->Header.InterfaceNo = kbd->iusb.addr & 0xff;
    hid->Header.ClientData = 0;
    hid->Header.Instance = 0;
    hid->Header.SeqNo = kbd->iusb.seq_no++;
    hid->Header.Key = kbd->iusb.key;

    p = pkt;
    for (i = 0; i<sizeof(IUSB_HEADER); i++, p++) {
        j = (j + *p) & 0xff;
    }

    hid->Header.HeaderCheckSum = j;
    hid->DataLen = 8;

    pkt[33+2] = 0;
    switch (keycode) {
    case 29: // Left Ctrl
        kbd->modifiers = up ? kbd->modifiers & 0xfe :
                              kbd->modifiers | 0x01;
        break;
    case 42: // Left Shift
        kbd->modifiers = up ? kbd->modifiers & 0xfd :
                              kbd->modifiers | 0x02;
        break;
    case 54: // Right Shift
        kbd->modifiers = up ? kbd->modifiers & 0xdf :
                              kbd->modifiers | 0x20;
        break;
    case 56: // Left Alt
        kbd->modifiers = up ? kbd->modifiers & 0xfb :
                              kbd->modifiers | 0x04;
        break;
    case 97: // Right Ctrl
        kbd->modifiers = up ? kbd->modifiers & 0xef :
                              kbd->modifiers | 0x10;
        break;
    case 100: // Right Alt
        kbd->modifiers = up ? kbd->modifiers & 0xbf :
                              kbd->modifiers | 0x40;
        break;
    case 199: // Home
        pkt[33+2] = up ? 0 : keycode2usbcode[102];
        break;
    case 200: // Up Arrow
        pkt[33+2] = up ? 0 : keycode2usbcode[103];
        break;
    case 201: // Page Up
        pkt[33+2] = up ? 0 : keycode2usbcode[104];
        break;
    case 203: // Left Arrow
        pkt[33+2] = up ? 0 : keycode2usbcode[105];
        break;
    case 205: // Right Arrow
        pkt[33+2] = up ? 0 : keycode2usbcode[106];
        break;
    case 207: // End
        pkt[33+2] = up ? 0 : keycode2usbcode[107];
        break;
    case 208: // Down Arrow
        pkt[33+2] = up ? 0 : keycode2usbcode[108];
        break;
    case 209: // Page Down
        pkt[33+2] = up ? 0 : keycode2usbcode[109];
        break;
    default:
        pkt[33+2] = (up || keycode > 255) ? 0 : keycode2usbcode[keycode];
    }
    pkt[33+1] = 1; // autoKeybreakModeOn
    pkt[33+0] = kbd->modifiers;

    printf("--- usb=%02x keycode=%02x up=%d m=%02x ", pkt[33+2], keycode, up, pkt[33+0]);

    printf(" ioctl=%d\n", ioctl(kbd->iusb.fd, USB_KEYBD_DATA, pkt));

    free(pkt);
}

static uint8_t kbd_get_leds(SpiceKbdInstance *sin)
{
    iUSBSpiceKbd *kbd = (iUSBSpiceKbd *)((void *)sin - sizeof(iUSBSpice));
    int i;
    int j = 0;
    uint8_t *p;
    IUSB_HID_PACKET *hid;
    uint8_t *pkt;

    pkt = calloc(sizeof(IUSB_HID_PACKET) + 4, 1);
    hid = (void *)pkt;
    memcpy(hid->Header.Signature, IUSB_SIG, sizeof(IUSB_SIG));
    hid->Header.Major = IUSB_MAJOR;
    hid->Header.Minor = IUSB_MINOR;
    hid->Header.HeaderLen = sizeof(IUSB_HEADER);
    hid->Header.HeaderCheckSum = 0;
    hid->Header.DataPktLen = 9;
    hid->Header.ServerCaps = 0;
    hid->Header.DeviceType = IUSB_DEVICE_KEYBD;
    hid->Header.Protocol = IUSB_PROTO_KEYBD_STATUS;
    hid->Header.Direction = FROM_REMOTE;
    hid->Header.DeviceNo = kbd->iusb.addr >> 8;
    hid->Header.InterfaceNo = kbd->iusb.addr & 0xff;
    hid->Header.ClientData = 0;
    hid->Header.Instance = 0;
    hid->Header.SeqNo = kbd->iusb.seq_no++;
    hid->Header.Key = kbd->iusb.key;

    p = pkt;
    for (i = 0; i<sizeof(IUSB_HEADER); i++, p++) {
        j = (j + *p) & 0xff;
    }

    hid->Header.HeaderCheckSum = j;
    hid->DataLen = 0;
    hid->Data = 0;

    if (ioctl(kbd->iusb.fd, USB_KEYBD_LED_NO_WAIT, pkt) == 0) {
        kbd->ledstate = 0;
//        printf("=== LED: %02x %d\n", hid->Data, hid->DataLen);
        if (hid->Data & 4) {
            kbd->ledstate |= SPICE_KEYBOARD_MODIFIER_FLAGS_SCROLL_LOCK;
        }
        if (hid->Data & 2) {
            kbd->ledstate |= SPICE_KEYBOARD_MODIFIER_FLAGS_NUM_LOCK;
        }
        if (hid->Data & 1) {
            kbd->ledstate |= SPICE_KEYBOARD_MODIFIER_FLAGS_CAPS_LOCK;
        }
        spice_server_kbd_leds(&kbd->sin, kbd->ledstate);
    }

    free(pkt);
    return kbd->ledstate;
}

static const SpiceKbdInterface kbd_interface = {
    .base.type          = SPICE_INTERFACE_KEYBOARD,
    .base.description   = "iUSB keyboard",
    .base.major_version = SPICE_INTERFACE_KEYBOARD_MAJOR,
    .base.minor_version = SPICE_INTERFACE_KEYBOARD_MINOR,
    .push_scan_freg     = kbd_push_key,
    .get_leds           = kbd_get_leds,
};

#define INPUT_BUTTON_LEFT       0
#define INPUT_BUTTON_MIDDLE     1
#define INPUT_BUTTON_RIGHT      2
#define INPUT_BUTTON_WHEEL_UP   3
#define INPUT_BUTTON_WHEEL_DOWN 4
#define INPUT_BUTTON_MAX        5

static void spice_update_buttons(iUSBSpicePointer *pointer,
                                 int wheel, uint32_t button_mask)
{
    static uint32_t bmap[INPUT_BUTTON_MAX] = {
        [INPUT_BUTTON_LEFT]        = 0x01,
        [INPUT_BUTTON_MIDDLE]      = 0x04,
        [INPUT_BUTTON_RIGHT]       = 0x02,
        [INPUT_BUTTON_WHEEL_UP]    = 0x10,
        [INPUT_BUTTON_WHEEL_DOWN]  = 0x20,
    };

    if (wheel < 0) {
        button_mask |= 0x10;
    }
    if (wheel > 0) {
        button_mask |= 0x20;
    }

    if (pointer->last_bmask == button_mask) {
        return;
    }
//    qemu_input_update_buttons(NULL, bmap, pointer->last_bmask, button_mask);
    pointer->last_bmask = button_mask;
}

static void mouse_motion(SpiceMouseInstance *sin, int dx, int dy, int dz,
                         uint32_t buttons_state)
{
    iUSBSpicePointer *pointer = (iUSBSpicePointer *)((void *)sin - sizeof(iUSBSpice));
    int i;
    int j = 0;
    uint8_t *p;
    IUSB_HID_PACKET *hid;
    uint8_t *pkt;

    spice_update_buttons(pointer, dz, buttons_state);
//    printf("MOUSE(%p): %d, %d  z=%d but=%d\n", pointer, dx, dy, dz, buttons_state);

    pkt = calloc(sizeof(IUSB_HID_PACKET) + 4, 1);
    hid = (void *)pkt;
    memcpy(hid->Header.Signature, IUSB_SIG, sizeof(IUSB_SIG));
    hid->Header.Major = IUSB_MAJOR;
    hid->Header.Minor = IUSB_MINOR;
    hid->Header.HeaderLen = sizeof(IUSB_HEADER);
    hid->Header.HeaderCheckSum = 0;
    hid->Header.DataPktLen = 9;
    hid->Header.ServerCaps = 0;
    hid->Header.DeviceType = IUSB_DEVICE_MOUSE;
    hid->Header.Protocol = IUSB_PROTO_MOUSE_DATA;
    hid->Header.Direction = FROM_REMOTE;
    hid->Header.DeviceNo = pointer->iusb.addr >> 8;
    hid->Header.InterfaceNo = pointer->iusb.addr & 0xff;
    hid->Header.ClientData = 0;
    hid->Header.Instance = 0;
    hid->Header.SeqNo = pointer->iusb.seq_no++;
    hid->Header.Key = pointer->iusb.key;

    p = pkt;
    for (i = 0; i<sizeof(IUSB_HEADER); i++, p++) {
        j = (j + *p) & 0xff;
    }

    hid->Header.HeaderCheckSum = j;
    hid->DataLen = 4;

    pkt[33+0] = pointer->last_bmask;
    pkt[33+1] = (uint8_t)dx; // putShort(x)
    pkt[33+2] = (uint8_t)dy; // putShort(y)
    pkt[33+3] = dz;

//    printf("--- %-3d (%-3d)   %-3d (%-3d)\n", (int8_t)pkt[33+1], dx, (int8_t)pkt[33+2], dy) ;//, pkt[33+3], pkt[33+0]);

//    printf(" ioctl=%d @ %d\n", , pointer->iusb.fd);
    if (ioctl(pointer->iusb.fd, USB_MOUSE_DATA, pkt) == 0) {
#if 0
        pointer->last_x += dx;
        pointer->last_y += dy;
        if (pointer->last_x < 0)
            pointer->last_x = 0;
        if (pointer->last_y < 0)
            pointer->last_y = 0;
#endif
    }

    free(pkt);
}

static void mouse_buttons(SpiceMouseInstance *sin, uint32_t buttons_state)
{
    iUSBSpicePointer *pointer = (iUSBSpicePointer *)((void *)sin - sizeof(iUSBSpice));
//    spice_update_buttons(pointer, 0, buttons_state);
    // FIXME: only for relative mouse mode:
    mouse_motion(sin, 0, 0, 0, buttons_state);
}

static const SpiceMouseInterface mouse_interface = {
    .base.type          = SPICE_INTERFACE_MOUSE,
    .base.description   = "iUSB mouse",
    .base.major_version = SPICE_INTERFACE_MOUSE_MAJOR,
    .base.minor_version = SPICE_INTERFACE_MOUSE_MINOR,
    .motion             = mouse_motion,
    .buttons            = mouse_buttons,
};

int main(void)
{
    Test *test;
    iUSBSpiceKbd *kbd;

    core = basic_event_loop_init();
    test = ast_new(core);

    kbd = g_malloc0(sizeof(iUSBSpiceKbd,));
    kbd->sin.base.sif = &kbd_interface.base;
    //test->pointer = g_malloc0(sizeof(iUSBSpicePointer));

    test->pointer.iusb.fd = kbd->iusb.fd = open(ASPEED_USB_DEV, O_RDONLY);
    if (kbd->iusb.fd < 0) {
        printf("unable to open iUSB device: %d", errno);
        return -1;
    }

    if (!iusb_request(&kbd->iusb, IUSB_DEVICE_KEYBD)) {
        printf("unable to lock keyboard");
        return -1;
    }

    spice_server_add_interface(test->server, &kbd->sin.base);

    test->pointer.sin.base.sif = &mouse_interface.base;

    if (!iusb_request(&test->pointer.iusb, IUSB_DEVICE_MOUSE)) {
        printf("unable to lock mouse");
        return -1;
    }

    spice_server_add_interface(test->server, &test->pointer.sin.base);

    iusb_set_mouse_mode(&test->pointer.iusb, spice_server_is_server_mouse(test->server));

    test->videocap_fd = open(ASPEED_ENCODER_VIDEOCAP_DEV, O_RDONLY);
    if (test->videocap_fd < 0) {
        printf("unable to open videocap device: %d", errno);
        return -1;
    }
    test->mmap = mmap(0, 0x404000, PROT_READ, MAP_SHARED, test->videocap_fd, 0);
    if (test->mmap == MAP_FAILED) {
        close(test->videocap_fd);
        printf("unable to mmap videocap device: %d", errno);
        return -1;
    }

    bzero(&test->ioc, sizeof(ASTCap_Ioctl));
    test->ioc.OpCode = ASTCAP_IOCTL_RESET_VIDEOENGINE;
    ioctl(test->videocap_fd, ASTCAP_IOCCMD, &test->ioc);

    bzero(&test->ioc, sizeof(ASTCap_Ioctl));
    test->ioc.OpCode = ASTCAP_IOCTL_START_CAPTURE;
    ioctl(test->videocap_fd, ASTCAP_IOCCMD, &test->ioc);

    ping_timer = core->timer_add(pinger, NULL);
    core->timer_start(ping_timer, ping_ms);

    basic_event_loop_mainloop();

    return 0;
}
