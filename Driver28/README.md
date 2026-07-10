# Driver28 — USB Device Driver

## What This Driver Does

A basic USB driver that detects when a specific USB device is plugged in (identified by Vendor ID and Product ID) and prints the device's interface and endpoint descriptors to the kernel log. When the device is unplugged, the disconnect callback fires.

> **Important**: This driver targets a specific USB device. Change `USB_VENDOR_ID` and `USB_PRODUCT_ID` to match your own USB device. Find these values with `lsusb`.

## Concepts Covered

- `struct usb_driver` — the USB driver registration structure
- `usb_register(&driver)` / `usb_deregister(&driver)` — register/deregister the USB driver
- `module_usb_driver(driver)` — shortcut macro (replaces `module_init`/`module_exit`)
- `struct usb_device_id` + `USB_DEVICE(vid, pid)` — specify which USB device this driver supports
- `MODULE_DEVICE_TABLE(usb, table)` — enables Linux hotplug/udev to auto-load the driver
- `probe` callback — called when a matching USB device is connected
- `disconnect` callback — called when the USB device is removed
- `struct usb_host_interface` — describes the current USB interface's alternate setting
- USB interface and endpoint descriptors

## Files

| File         | Description |
|--------------|-------------|
| `driver28.c` | Module source code |
| `Makefile`   | Build configuration |

## Customization (Required)

Before loading this driver, update the Vendor and Product IDs to match your device:

```c
#define USB_VENDOR_ID   ( 0x22d9 )   // Change to your device's Vendor ID
#define USB_PRODUCT_ID  ( 0x2764 )   // Change to your device's Product ID
```

Find your device's IDs:
```bash
lsusb
# Example output:
# Bus 001 Device 003: ID 0930:6544 Toshiba Corp. TransMemory-Mx  <- 0930 is VID, 6544 is PID
```

## Build & Run

```bash
# Edit USB_VENDOR_ID and USB_PRODUCT_ID first!
make

sudo insmod driver28.ko
dmesg | tail -5

# Plug in the USB device
dmesg | tail -30    # Should print interface + endpoint descriptors

# Unplug the USB device
dmesg | tail -5     # Should print "USB Driver Disconnected"

sudo rmmod driver28
```

## Two Initialization Methods

The driver supports two methods controlled by `IS_NEW_METHOD_USED`:

```c
#define IS_NEW_METHOD_USED ( 1 )
```

| Value | Method | Description |
|-------|--------|-------------|
| `0`   | New    | Uses `module_usb_driver(etx_usb_driver)` macro |
| `1`   | Old    | Uses explicit `module_init` / `module_exit` with `usb_register` / `usb_deregister` |

> The names "new method" and "old method" in the comments are **reversed** — `module_usb_driver` is the newer, preferred approach. This is a naming inconsistency in the code.

## Code Notes / Issues

- **`IS_NEW_METHOD_USED = 1` uses the "old" manual init/exit method**: The comment says "new method" uses `module_usb_driver`, but the macro is set to `1` which selects the manual path. The naming is opposite to what you'd expect.
- **The probe function always returns 0**: In a real driver, `etx_usb_probe` would claim the interface by calling `usb_set_intfdata` and setting up URBs for data transfer. Here it only prints descriptors.
- **No data transfer**: This driver only detects the device — it doesn't read from or write to it. Adding actual data transfer requires URB (USB Request Block) submission.
- **`dev_info` vs `pr_info`**: The probe and disconnect functions use `dev_info(&interface->dev, ...)` which is preferred in device drivers — it prepends the device name to the log message, making it easier to identify which device generated the message.
