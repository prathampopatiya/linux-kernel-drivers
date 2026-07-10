# Driver17 — Mutex (Thread Synchronization)

## What This Driver Does

Creates two kernel threads that both increment a shared global variable. A mutex ensures only one thread modifies the variable at a time, preventing a race condition. This is a fundamental synchronization pattern.

## What is a Mutex?

A mutex (mutual exclusion lock) allows only one thread to hold it at a time. If a thread tries to lock an already-held mutex, it **sleeps** until the mutex is released. This makes mutexes suitable only for process context (kernel threads), not for interrupt handlers.

## Concepts Covered

- `struct mutex` — the mutex data structure
- `mutex_init(&mutex)` — dynamically initialize a mutex
- `DEFINE_MUTEX(name)` — statically declare and initialize a mutex
- `mutex_lock(&mutex)` — acquire the mutex (sleeps if unavailable)
- `mutex_unlock(&mutex)` — release the mutex
- `mutex_lock_interruptible` — like `mutex_lock` but can be interrupted by signals
- Race conditions and why they are dangerous

## Files

| File         | Description |
|--------------|-------------|
| `driver17.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver17.ko
dmesg | tail -5

# Both threads increment etx_global_var every second with mutex protection
sleep 5
dmesg | tail -15

sudo rmmod driver17
```

## How It Works

```
Thread 1 (thread_func1)          Thread 2 (thread_func2)
    └─> mutex_lock(&etx_mutex)       └─> [blocks here if mutex is held]
    └─> etx_global_var++
    └─> pr_info(value)
    └─> mutex_unlock(&etx_mutex)  ──> Thread 2 now acquires mutex
    └─> msleep(1000)                  └─> etx_global_var++
                                      └─> pr_info(value)
                                      └─> mutex_unlock(&etx_mutex)
```

## Code Notes / Issues

- **`mutex_init` not called**: The `struct mutex etx_mutex` is declared globally (line 57) but `mutex_init(&etx_mutex)` is never called in `etx_driver_init`. The mutex is zero-initialized by the BSS, which **may** work on some kernel versions, but this is relying on undefined behavior. You must explicitly call `mutex_init(&etx_mutex)` (or use `DEFINE_MUTEX(etx_mutex)` at declaration).
- **Thread error check for `etx_thread1` is wrong** (line 202): `if(etx_thread1)` — `kthread_run` returns `ERR_PTR(...)` on failure, not NULL. This check should be `if(!IS_ERR(etx_thread1))`.
- **`kthread_stop` without `IS_ERR` check in exit**: If thread creation failed, calling `kthread_stop(etx_thread)` on an ERR_PTR will crash. Check with `IS_ERR_OR_NULL` before stopping.
