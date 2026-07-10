# Linux Kernel Drivers

A progressive series of Linux kernel module examples, built while following the [EmbeTronicX Linux Device Driver Tutorials](https://embetronicx.com/linux-device-driver-tutorials/). Some drivers use newer kernel APIs compared to the original tutorials.

## Prerequisites

- Linux kernel headers installed (`linux-headers-$(uname -r)`)
- `make`, `gcc`, and basic build tools
- Root / `sudo` access to load and unload kernel modules

## How to Build and Use Any Driver

Every driver directory contains a `Makefile`. The general workflow is:

```bash
# Enter the driver directory
cd Driver-X/

# Compile the module
make

# Load the module
sudo insmod driverX.ko

# Check kernel logs
dmesg | tail -20

# Unload the module
sudo rmmod driverX
```

## Driver Series Overview

| Directory  | Topic |
|------------|-------|
| [Driver-1](./Driver-1)   | Hello World – First kernel module |
| [Driver-2](./Driver-2)   | Module Parameters |
| [Driver-3](./Driver-3)   | Device Numbers – Static & Dynamic allocation |
| [Driver-4](./Driver-4)   | Device File – Auto-creation via sysfs |
| [Driver-5](./Driver-5)   | File Operations – open / read / write / release |
| [Driver-6](./Driver-6)   | Kernel Memory – kmalloc + copy_to/from_user |
| [Driver-7](./Driver-7)   | IOCTL – Kernel-userspace control interface |
| [Driver-8](./Driver-8)   | Procfs – /proc filesystem interface |
| [Driver-9](./Driver-9)   | Wait Queue – Static & Dynamic methods |
| [Driver10](./Driver10)   | Sysfs – Kernel object attributes |
| [Driver11](./Driver11)   | Interrupt Handling – Shared IRQ + Sysfs |
| [Driver12](./Driver12)   | Workqueue – Static method |
| [Driver13](./Driver13)   | Workqueue – Custom/Own workqueue |
| [Driver14](./Driver14)   | Linked List – Kernel linked list in a driver |
| [Driver15](./Driver15)   | Kernel Threads – kthread_create + kthread_run |
| [Driver16](./Driver16)   | Tasklets – Bottom-half interrupt handling |
| [Driver17](./Driver17)   | Mutex – Thread synchronization |
| [Driver18](./Driver18)   | Spinlock – Busy-wait thread synchronization |
| [Driver19](./Driver19)   | Read-Write Spinlock |
| [Driver20](./Driver20)   | Signals – Kernel-to-userspace signal delivery |
| [Driver21](./Driver21)   | Kernel Timer – Periodic timer using jiffies |
| [Driver22](./Driver22)   | High Resolution Timer (hrtimer) |
| [Driver23](./Driver23)   | Completion – Synchronization primitive |
| [Driver24](./Driver24)   | Exported Symbols – Sharing between modules |
| [Driver25](./Driver25)   | Atomic Variables & Bit Operations |
| [Driver26](./Driver26)   | Seqlock – Writer-priority read-write lock |
| [Driver27](./Driver27)   | Miscellaneous Device Driver |
| [Driver28](./Driver28)   | USB Driver – Basic USB device detection |

## License

GPL v2 – See [LICENSE](./LICENSE)

## Author

Pratham Popatiya <prathampopatiya17@gmail.com>
