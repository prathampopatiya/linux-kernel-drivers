# Driver-5 — File Operations (open / read / write / release)

## What This Driver Does

Adds a character device with fully registered file operations. After loading this module, userspace programs can `open()`, `read()`, `write()`, and `close()` the device file `/dev/etx_Device`. Each call prints a message to the kernel log.

## Concepts Covered

- `struct file_operations` — the table that maps system calls to driver functions
- `cdev_init` / `cdev_add` — registering a character device with the kernel
- `cdev_del` — removing the character device during cleanup
- `struct cdev` — the kernel structure representing a character device
- `etx_open`, `etx_release`, `etx_read`, `etx_write` — standard file operation callbacks

## Files

| File        | Description |
|-------------|-------------|
| `driver5.c` | Module source code |
| `Makefile`  | Build configuration |

## Build & Run

```bash
make
sudo insmod driver5.ko
dmesg | tail -5

# Interact with the device
cat /dev/etx_Device       # Triggers etx_read (returns 0 bytes, prints log)
echo "hello" > /dev/etx_Device  # Triggers etx_write

dmesg | tail -10
sudo rmmod driver5
```

## File Operations Structure

```c
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = etx_open,
    .release = etx_release,
    .read    = etx_read,
    .write   = etx_write,
};
```

## Code Notes / Issues

- **`etx_read` returns 0**: This is correct for this demo (no actual data to return), but `cat` will loop until it gets EOF (0 bytes returned). This is intentional here.
- **Error path bug**: When `cdev_add` fails, the code jumps to `r_class` but the class has not been created yet at that point. The label order is slightly off — `cdev_add` failure should jump to a label that only calls `unregister_chrdev_region`. As written, both `cdev_add` failure and `class_create` failure jump to `r_class`, which works, but it would be cleaner to have separate labels.
- **`class_create` uses new API**: No `THIS_MODULE` argument — correct for kernel ≥ 6.4.
- **Typo in release function**: Line 38 — `"Driver Release fucntion called"` — "fucntion" should be "function".
