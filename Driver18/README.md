# Driver18 — Spinlock (Busy-Wait Synchronization)

## What This Driver Does

Same setup as Driver17 (two threads incrementing a shared variable) but uses a **spinlock** instead of a mutex. A spinlock does not sleep — it keeps the CPU busy-waiting ("spinning") until the lock is free. This makes spinlocks safe to use in interrupt context.

## Spinlock vs Mutex

| | Spinlock | Mutex |
|---|---|---|
| Waiting behavior | Busy-waits (spins) | Sleeps |
| Usable in IRQ context | ✅ Yes | ❌ No |
| CPU usage while waiting | High | Low |
| Best for | Very short critical sections | Longer operations |

## Concepts Covered

- `DEFINE_SPINLOCK(name)` — statically declare and initialize a spinlock
- `spinlock_t` + `spin_lock_init(&lock)` — dynamic initialization
- `spin_lock(&lock)` / `spin_unlock(&lock)` — acquire/release in process context
- `spin_lock_irq` / `spin_unlock_irq` — disable IRQs while holding the lock (for IRQ-shared data)
- `spin_lock_bh` / `spin_unlock_bh` — disable software interrupts (for softirq-shared data)

## Files

| File         | Description |
|--------------|-------------|
| `driver18.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver18.ko
dmesg | tail -5

# Threads spin and increment the variable every second
sleep 5
dmesg | tail -15

sudo rmmod driver18
```

## Code Notes / Issues

- **`msleep(1000)` inside the critical section is wrong**: The spinlock is acquired, the variable is incremented, then `mutex_unlock` (from the comment copy in the file) — actually looking at the code, `msleep` is called **after** `spin_unlock`. This is correct. Never call `msleep` or any sleeping function while holding a spinlock.
- **Same thread error check issue as Driver17**: `if(etx_thread1)` on line 234 should be `if(!IS_ERR(etx_thread1))`.
- **Same missing exit safety as Driver17**: `kthread_stop` in exit should check `IS_ERR_OR_NULL` on both thread pointers.
- **`#include <linux/mutex.h>`** is included (line 13) but no mutex is used — only the spinlock. This leftover include from the mutex driver can be removed.
- **MODULE_DESCRIPTION is copy-pasted**: The description says "Simple device Driver allocating memory and reading from/to userspace" — should be updated to describe spinlocks.
