# Driver-8 — Procfs (The /proc Filesystem)

## What This Driver Does

Creates a directory `/proc/etx/` containing a file `/proc/etx/etx_proc`. This procfs entry lets a userspace program read from and write to the driver through the `/proc` filesystem, which is an alternative communication channel to the device file.

The driver also includes IOCTL support (carried over from Driver-7).

## Concepts Covered

- `proc_mkdir(name, parent)` — create a directory inside `/proc`
- `proc_create(name, mode, parent, fops)` — create a file inside `/proc`
- `proc_remove(entry)` — removes a proc directory and all its children
- `struct proc_ops` — the modern (kernel ≥ 5.6) replacement for `struct file_operations` in procfs
- How the `len` toggle trick works in `read_proc` to signal EOF to userspace

## Files

| File        | Description |
|-------------|-------------|
| `driver8.c` | Module source code |
| `Makefile`  | Build configuration |

## Build & Run

```bash
make
sudo insmod driver8.ko
dmesg | tail -5

# Read from /proc
cat /proc/etx/etx_proc       # Prints "try_proc_array"

# Write to /proc
echo "new message" | sudo tee /proc/etx/etx_proc

# Read it back
cat /proc/etx/etx_proc

sudo rmmod driver8
```

## Kernel Version Compatibility

This driver uses a compile-time version check:

```c
#define LINUX_KERNEL_VERSION 700

#if (LINUX_KERNEL_VERSION > 505)
    // Use struct proc_ops (kernel >= 5.6+)
#else
    // Use struct file_operations for proc (older kernels)
#endif
```

> **Note**: `LINUX_KERNEL_VERSION` is a custom `#define` set to `700`, which is not the actual kernel version. This means the `struct proc_ops` branch is **always** taken regardless of the real kernel version. This is fine on modern kernels but is fragile. A better approach would use the kernel's own `LINUX_VERSION_CODE` macro.

## API Reference

```c
struct proc_ops proc_fops = {
    .proc_open    = open_proc,
    .proc_read    = read_proc,
    .proc_write   = write_proc,
    .proc_release = release_proc,
};
```

## Code Notes / Issues

- **`LINUX_KERNEL_VERSION` is misleading**: The macro is a fixed value (`700`), not the actual running kernel version. Use `#include <linux/version.h>` and `LINUX_VERSION_CODE` / `KERNEL_VERSION(5,6,0)` for a real version check.
- **`read_proc` copies 20 bytes unconditionally**: The function always copies `etx_array[20]` regardless of the `length` argument. If the user buffer is smaller than 20 bytes, this will overflow the user buffer.
- **`etx_array` size and write boundary**: `write_proc` accepts `length` bytes from the user but `etx_array` is fixed at 20 characters. Writing more than 20 bytes will cause a buffer overflow in the kernel.
- **`proc_create` mode `0666`**: Creating a proc file world-writable is a security concern. Use `0644` for production code.
- **Typo in log**: `"proc file opend....."` — "opend" should be "opened".
