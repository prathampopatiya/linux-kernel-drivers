# Driver15 — Kernel Threads (kthread_create + kthread_run)

## What This Driver Does

Creates a kernel thread that runs a loop, printing a counter and sleeping for 1 second per iteration. The thread is started with `wake_up_process` after creation and is stopped cleanly when the module is unloaded.

## Concepts Covered

- `kthread_create(fn, data, name)` — create a kernel thread (but does NOT start it)
- `wake_up_process(task)` — start a thread created with `kthread_create`
- `kthread_run(fn, data, name)` — shortcut that creates and starts the thread in one call
- `kthread_should_stop()` — returns true when `kthread_stop` has been called; use this as the loop condition to allow clean exit
- `kthread_stop(task)` — signals the thread to stop and waits for it to exit
- `IS_ERR(ptr)` / `IS_ERR_OR_NULL(ptr)` — safe way to check kernel pointer errors
- `msleep(ms)` — sleep for milliseconds inside a kernel thread

## Files

| File         | Description |
|--------------|-------------|
| `driver15.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver15.ko
dmesg | tail -5

# Wait a few seconds — thread logs every 1 second
sleep 5
dmesg | tail -10

# Unload — thread stops cleanly
sudo rmmod driver15
dmesg | tail -3
```

## Thread Lifecycle

```
[Module Load]
  etx_driver_init()
    └─> kthread_create(thread_func, NULL, "Etx_Thread")
    └─> wake_up_process(etx_thread)
          └─> thread_func: loop while !kthread_should_stop()
                             └─> pr_info, msleep(1000)

[Module Unload]
  etx_driver_exit()
    └─> kthread_stop(etx_thread)   -- sets should_stop flag and waits
          └─> thread_func: kthread_should_stop() returns true → loop exits → return 0
```

## Code Notes / Issues

- **`r_unregister` label is never reached**: The error path label `r_unregister` (line 209) is defined after `r_cdev`. The `r_cdev` goto goes to that label, but `r_cdev` itself calls `cdev_del` then falls through to `r_unregister`. This pattern works but is confusing. Many drivers use cleaner chained goto labels.
- **Thread not stopped before device cleanup on some error paths**: If `kthread_create` fails, the code falls into the device cleanup path but since the thread was never started, this is fine. However, if the thread creation error handling is extended, be careful about the cleanup order.
- **`kthread_run` alternative is shown in `#if 0` block**: The commented block on lines 182–192 shows the simpler `kthread_run` approach. This is the preferred method for new code — it's cleaner than `kthread_create` + `wake_up_process`.
