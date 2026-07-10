# Driver25 — Atomic Variables & Bit Operations

## What This Driver Does

Uses **atomic variables** and **atomic bit operations** to safely share data between two kernel threads — without any locks. Atomic operations are guaranteed to be indivisible at the hardware level, making them safe for concurrent access without mutexes or spinlocks.

## Concepts Covered

- `atomic_t` — kernel atomic integer type
- `ATOMIC_INIT(val)` — initialize an atomic variable at declaration
- `atomic_inc(&var)` — atomically increment
- `atomic_read(&var)` — atomically read the value
- `atomic_set(&var, val)` — atomically set the value
- `test_and_change_bit(nr, addr)` — atomically read a bit and flip it, returns the old value
- When to use atomics vs mutexes vs spinlocks

## Files

| File         | Description |
|--------------|-------------|
| `driver25.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver25.ko
dmesg | tail -5

# Both threads run concurrently, safely modifying atomic variable
sleep 5
dmesg | tail -15

sudo rmmod driver25
```

## How It Works

```
Thread1 and Thread2 both run every 1 second:
  └─> atomic_inc(&etx_global_variable)   -- safe, no lock needed
  └─> test_and_change_bit(1, &bit_check) -- atomically flip bit 1, get old value
  └─> pr_info(atomic_read(&etx_global_variable), prev_value)
```

## Code Notes / Issues

- **`pr_err` used for success messages** (lines 131 and 139): `pr_err("[+] Thread1 created and running successfully\n")` — success messages should use `pr_info`, not `pr_err`. `pr_err` implies an error condition.
- **Thread error check uses `if(etx_thread1)` (line 130)**: `kthread_run` returns `ERR_PTR` on failure, not NULL. The check should be `if(!IS_ERR(etx_thread1))`. Same issue on line 138 for `etx_thread2`.
- **No `kthread_stop` safety check in exit**: If either thread creation failed, calling `kthread_stop` on the invalid pointer will panic. Add `IS_ERR_OR_NULL` checks in the exit function.
- **`bit_check` is not volatile or atomic**: `bit_check` is a plain `unsigned int` but is accessed with `test_and_change_bit`. The bit operation itself is atomic, but the compiler may optimize away the value if it doesn't see it as externally modifiable. Adding `volatile` or declaring it with a proper type would be safer.
- **`cdev_del` called in `r_class` cleanup**: In the error path at `r_class`, `cdev_del` is called, but `cdev_add` may have failed and was never called. `cdev_del` on an uninitialized/unadded cdev may be harmless but should be documented.
