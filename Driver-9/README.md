# Driver-9 — Wait Queues (Static & Dynamic Methods)

## What This Driver Does

Demonstrates how a kernel thread can sleep and wait for an event triggered by a device file operation. When a userspace program reads the device file, it wakes up a sleeping kernel thread. Two implementations are provided in parallel:

| File           | Method  | How wait queue is created |
|----------------|---------|--------------------------|
| `driver9.c`    | Static  | `DECLARE_WAIT_QUEUE_HEAD(wait_queue_etx)` |
| `driver9Dyn.c` | Dynamic | `wait_queue_head_t wait_queue_etx;` + `init_waitqueue_head(...)` |

## Concepts Covered

- `DECLARE_WAIT_QUEUE_HEAD(name)` — statically declare and initialize a wait queue
- `init_waitqueue_head(wq)` — dynamically initialize a wait queue
- `wait_event_interruptible(wq, condition)` — put the current thread to sleep until `condition` is true, or a signal arrives
- `wake_up_interruptible(wq)` — wake up one waiting thread from the queue
- `kthread_create` + `wake_up_process` — creating and starting a kernel thread
- Using a flag variable as the wait condition to avoid spurious wakeups

## Files

| File            | Description |
|-----------------|-------------|
| `driver9.c`     | Static wait queue method |
| `driver9Dyn.c`  | Dynamic wait queue method |
| `Makefile`      | Builds both modules |

## Build & Run

```bash
make

# Static method
sudo insmod driver9.ko
dmesg | tail -5         # Thread starts and waits

cat /dev/etx_wait_device   # Triggers wake_up_interruptible
dmesg | tail -5             # Thread wakes up and prints event count

sudo rmmod driver9

# Dynamic method
sudo insmod driver9Dyn.ko
# Same usage as above
sudo rmmod driver9Dyn
```

## How It Works

```
[Module Load]
  └─> kthread_create → kernel thread starts → waits at wait_event_interruptible

[User reads /dev/etx_wait_device]
  └─> etx_read() sets wait_queue_flag = 1
  └─> wake_up_interruptible() → thread wakes
  └─> thread prints read count, resets flag, goes back to sleep

[Module Unload]
  └─> etx_driver_exit() sets wait_queue_flag = 2
  └─> wake_up_interruptible() → thread sees flag == 2, exits
```

## Code Notes / Issues

- **`do_exit(0)` after the while loop** (driver9.c, line 70): The `while(1)` loop never terminates normally, so `do_exit(0)` is unreachable dead code. It's harmless but should be removed for clarity. `driver9Dyn.c` correctly omits it.
- **`etx_cdev.ops = &fops` is redundant** (both files, after `cdev_init`): `cdev_init` already sets `.ops` internally. The manual assignment on the next line is redundant but not harmful.
- **No `kthread_stop` in exit**: Neither driver calls `kthread_stop` before exiting. The thread signals itself to stop by checking `wait_queue_flag == 2`, but if the thread is not running (e.g., it already exited), `wake_up_interruptible` is still safe. However, not calling `kthread_stop` means the exit doesn't wait for the thread to finish, which could be a race. Add `kthread_stop(wait_thread)` after the wake_up in the exit function.
- **MODULE_DESCRIPTION typo** in `driver9Dyn.c`: The description says "Static method" but this is the dynamic variant.
