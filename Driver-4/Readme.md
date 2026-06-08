## Device Files

The device file allows transparent communication between user-space applications and hardware.

They are not normal “files”, but look like files from the program’s point of view: you can read from them, write to them, mmap() onto them, and so forth. When you access such a device “file,” the kernel recognizes the I/O request and passes it to a device driver, which performs some operations, such as reading data from a serial port or sending data to hardware.

Device files (although inappropriately named, we will continue to use this term) provide a convenient
way to access system resources without requiring the application programmer to know how the underlying device works. Under Linux, as with most Unix systems, device drivers themselves are part of the kernel.

---
## Creating a Device file
1. Manaully
2. Automatically 

### Manaully Method
```bash
mknod -m <permissons> <name> <device type> <major> <minor>

# eg

sudo mknod -m 666 /dev/etc_device c 246 0
```

