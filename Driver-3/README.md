# Driver-3 — Device Numbers (Static & Dynamic Allocation)

This directory contains two sub-projects demonstrating the two ways to allocate a character device number in Linux.

## Sub-directories

| Directory | Method | API Used |
|-----------|--------|----------|
| `Static/` | You pick the major/minor number | `register_chrdev_region` |
| `Dynamic/` | The kernel picks a free major number | `alloc_chrdev_region` |

---

## Static/ — Static Device Number Allocation

### What It Does

Hardcodes major number **335**, minor **0** using `MKDEV(335, 0)` and registers it with the kernel.

### Key APIs

- `MKDEV(major, minor)` — packs a major and minor number into a `dev_t`
- `register_chrdev_region(dev, count, name)` — registers a pre-chosen device number range
- `unregister_chrdev_region(dev, count)` — releases the range on cleanup
- `MAJOR(dev)` / `MINOR(dev)` — macros to extract parts from `dev_t`

### Build & Run

```bash
cd Static/
make
sudo insmod driver3.ko
dmesg | tail -5          # Shows Major=335 Minor=0
cat /proc/devices        # Should list "New Device" at major 335
sudo rmmod driver3
```

### Issues

- **No error check** on `register_chrdev_region`. If major 335 is already taken, the registration silently fails. Add a return value check.
- **Typos in comments**: "mjaor" (line 6) and "wirh" should be "major" and "with".
- **Static major numbers are not portable.** Major 335 may already be used on your system or a different kernel version. Dynamic allocation (see `Dynamic/`) is always preferred.

---

## Dynamic/ — Dynamic Device Number Allocation

### What It Does

Asks the kernel to assign a free major number using `alloc_chrdev_region`, then prints it to the kernel log.

### Key APIs

- `alloc_chrdev_region(&dev, baseminor, count, name)` — dynamically allocates a device number
- `unregister_chrdev_region(dev, count)` — frees the allocated range on exit

### Build & Run

```bash
cd Dynamic/
make
sudo insmod driver3.ko
dmesg | tail -5          # Prints the dynamically assigned Major and Minor numbers
cat /proc/devices        # Lists "DynamicDevv" with its assigned major number
sudo rmmod driver3
```

### Issues

- **Typo in comment**: "wirh" should be "with" (line 7).
- The error path returns `-1` — prefer `-EBUSY` or the actual return value from `alloc_chrdev_region` for better debugging.
