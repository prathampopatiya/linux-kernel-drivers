# Driver10 — Sysfs Interface (kobject + attributes)

## What This Driver Does

Creates a sysfs entry at `/sys/kernel/etx_sysfs/etx_value`. This file can be read and written from userspace using standard shell tools, allowing a clean, file-based interface to expose driver state or configuration without using ioctls or procfs.

## Concepts Covered

- `struct kobject` — the fundamental object at the heart of the sysfs model
- `kobject_create_and_add(name, parent)` — create a directory in sysfs
- `sysfs_create_file(kobj, attr)` — create an attribute file in the sysfs directory
- `sysfs_remove_file` / `kobject_put` — cleanup
- `struct kobj_attribute` + `__ATTR(name, permissions, show, store)` — define a sysfs attribute with its read/write callbacks
- `kernel_kobj` — places the directory under `/sys/kernel/`

## Files

| File         | Description |
|--------------|-------------|
| `driver10.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver10.ko
dmesg | tail -5

# Read the current value (default 0)
cat /sys/kernel/etx_sysfs/etx_value

# Write a new value
echo 42 | sudo tee /sys/kernel/etx_sysfs/etx_value

# Read it back
cat /sys/kernel/etx_sysfs/etx_value

sudo rmmod driver10
```

## Sysfs Attribute Definition

```c
struct kobj_attribute etx_attr = __ATTR(etx_value, 0660, sysfs_show, sysfs_store);
```

- **`sysfs_show`** — called on `cat`, uses `sprintf` to write the value into the sysfs read buffer
- **`sysfs_store`** — called on `echo`, uses `sscanf` to parse the written value

## Code Notes / Issues

- **`sprintf` in sysfs_show**: The `sysfs_show` callback uses `sprintf(buf, "%d", etx_value)`. On modern kernels the preferred function is `sysfs_emit(buf, "%d\n", etx_value)` which is bounds-safe and adds the required trailing newline.
- **Error handling in `r_sysfs` path**: When `sysfs_create_file` fails, the error handler calls `sysfs_remove_file(kernel_kobj, ...)` — but the file was never created. This is harmless but noisy. It should call `kobject_put(kobj_ref)` only.
- **`volatile int etx_value`**: Using `volatile` in kernel code is generally discouraged and does not replace proper locking. If multiple threads access this value, a spinlock or atomic type should be used.
- **Missing `cdev_del` in `r_class` path**: In the init error path at `r_class`, `cdev_del` is not called before `unregister_chrdev_region`. Since `cdev_add` hadn't succeeded at that point, this is technically correct, but the cleanup order comment should clarify this.
