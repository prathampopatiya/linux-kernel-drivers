# Driver-4 — Automatic Device File Creation

## What This Driver Does

Builds on Driver-3 by automatically creating the device file under `/dev/etx_device` using the kernel's sysfs/udev infrastructure. Previously, you had to manually run `mknod`; here the kernel does it for you.

## Concepts Covered

- `class_create(name)` — creates a device class visible under `/sys/class/`
- `device_create(class, parent, dev, drvdata, name)` — creates the `/dev/` entry
- `device_destroy` / `class_destroy` — cleanup on module removal
- Proper error handling with `goto` labels to unwind in reverse order

## Files

| File        | Description |
|-------------|-------------|
| `driver4.c` | Module source code |
| `Makefile`  | Build configuration |
| `Readme.md` | Original notes (this README supersedes it) |

## Build & Run

```bash
make
sudo insmod driver4.ko
ls /dev/etx_device        # Device file should appear automatically
ls /sys/class/etx_class/  # Class directory in sysfs
dmesg | tail -10
sudo rmmod driver4
ls /dev/etx_device        # Should be gone
```

## API Reference

```c
// Create a class (visible in /sys/class/)
struct class *class_create(const char *name);

// Create the device and /dev/ entry
struct device *device_create(struct class *cls, struct device *parent,
                              dev_t devt, void *drvdata, const char *fmt, ...);
```

## Code Notes / Issues

- **`class_create` API change**: This driver correctly uses the modern single-argument `class_create("etx_class")` (no `THIS_MODULE` argument), which is correct for kernels ≥ 6.4. Older tutorials show `class_create(THIS_MODULE, "name")` — do **not** use that form on modern kernels.
- **No `cdev` registered**: This driver allocates a device number and creates the `/dev/` entry, but does not call `cdev_init`/`cdev_add`. So the device file exists but cannot be opened yet. That is added in Driver-5.
- The `goto r_device` in the error path correctly calls `class_destroy`, but there is no label between `r_class` and `r_device` for the `cdev_del` (since no cdev is registered here — this is intentional in the series).
