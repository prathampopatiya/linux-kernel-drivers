# Driver-1 — Hello World Kernel Module

## What This Driver Does

This is the simplest possible Linux kernel module. It prints a message to the kernel log when it is loaded and another message when it is removed. No device file or hardware is involved.

## Concepts Covered

- `module_init` / `module_exit` — entry and exit points of a kernel module
- `printk` / `pr_info` — printing to the kernel ring buffer (viewed with `dmesg`)
- `MODULE_LICENSE`, `MODULE_AUTHOR`, `MODULE_DESCRIPTION`, `MODULE_VERSION` — metadata macros
- `__init` / `__exit` — hints to the kernel to free the function memory after use

## Files

| File        | Description |
|-------------|-------------|
| `driver1.c` | Module source code |
| `Makefile`  | Build configuration |

## Build & Run

```bash
make
sudo insmod driver1.ko
dmesg | tail -5
sudo rmmod driver1
dmesg | tail -3
```

## Expected Output (`dmesg`)

```
First Kernel Driver
This is simple driver
kernel module inserted successfully
...
Kerned module removed successfully
```

## Code Notes / Issues

- **Typo in exit message**: Line 13 — `"Kerned module removed successfully"` should be `"Kernel module removed successfully"`.
- `printk` with `KERN_INFO` and `pr_info` both write to the kernel log. They are equivalent; using `pr_info` consistently is the modern preference.
