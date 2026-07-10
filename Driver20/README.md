# Driver20 — Sending Signals from Kernel to Userspace

## What This Driver Does

Demonstrates how a kernel driver can send a real-time signal to a userspace process. The userspace application registers itself with the driver via IOCTL, and when IRQ 11 fires, the kernel sends signal `SIGETX` (signal 51) to the registered process.

This is one of the few mechanisms for asynchronous kernel-to-user notification without userspace polling.

## Concepts Covered

- `struct kernel_siginfo` — the kernel-side signal information structure
- `send_sig_info(signo, &info, task)` — send a signal to a specific task/process
- `get_current()` — get the `task_struct` of the currently running process
- `SIGETX` — a real-time signal (value 51) chosen to avoid conflicts with standard signals
- `REG_CURRENT_TASK` — custom IOCTL command to register the userspace process
- How the userspace app must install a signal handler for `SIGETX` before calling the IOCTL

## Files

| File         | Description |
|--------------|-------------|
| `driver20.c` | Kernel module source |
| `test.c`     | Userspace app that registers and receives the signal |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make

sudo insmod driver20.ko
dmesg | tail -5

# Compile and run the userspace test
gcc -o test test.c
sudo ./test        # Registers with driver and waits for signal

# In another terminal, trigger the IRQ
# Reading the device fires int $0x3b
cat /dev/etx_device    # Note: etx_read uses asm("int $0x3b")

sudo rmmod driver20
```

## Signal Flow

```
[Userspace: test.c]
    └─> install signal handler for SIGETX
    └─> open /dev/etx_device
    └─> ioctl(REG_CURRENT_TASK)    -- registers task_struct with driver

[IRQ 11 fires in kernel]
    └─> irq_handler()
          └─> send_sig_info(SIGETX, &info, task)

[Userspace signal handler fires]
    └─> Receives SIGETX, processes the event
```

## Code Notes / Issues

- **`asm("int $0x3b")` in `etx_read`** (line 96): The read function triggers a software interrupt to simulate the IRQ. This is x86-only and the same IRQ mismatch issue as Driver14 applies (see Driver14 notes).
- **`task` global pointer is not locked**: The global `static struct task_struct *task` is accessed from both the ioctl handler and the IRQ handler without any synchronization. If the IRQ fires while the process is closing the file (which sets `task = NULL`), there is a race condition. Use RCU or a spinlock to protect access to `task`.
- **Error goto logic has a missing `cdev_del`**: In the `r_class` error label (line 151), `cdev_del` is not called even though `cdev_add` may have succeeded.
- **`SIGETX = 51`**: Real-time signals in Linux start at `SIGRTMIN`. Signal 51 is in the real-time range on most Linux systems. The userspace test must use the same signal number.
