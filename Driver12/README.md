# Driver12 — Workqueue (Static Method)

## What This Driver Does

Demonstrates the **static workqueue** method. A work item is declared at compile time using `DECLARE_WORK`. When IRQ 11 fires, the interrupt handler queues the work item to the kernel's shared workqueue. The work function then runs in process context (not interrupt context), where sleeping is allowed.

## Why Workqueues?

Interrupt handlers run in atomic context and must return quickly. For longer tasks (like accessing slow peripherals, doing memory allocation with `GFP_KERNEL`, etc.), a workqueue defers the heavy work to a kernel thread running in process context.

## Concepts Covered

- `DECLARE_WORK(name, func)` — statically declare a work item
- `schedule_work(&work)` — queue work to the default system workqueue
- `queue_work(wq, &work)` — queue work to a specific workqueue
- Difference between **top half** (interrupt handler) and **bottom half** (workqueue function)
- The static workqueue uses the kernel's shared `system_wq` — no need to create your own

## Files

| File         | Description |
|--------------|-------------|
| `driver12.c` | Module source with static workqueue |
| `driver12Dyn.c` | Alternative using a custom/own workqueue |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver12.ko    # or driver12Dyn.ko
dmesg | tail -5

# The workqueue function runs whenever IRQ 11 fires
cat /proc/interrupts | grep etx

sudo rmmod driver12
```

## How It Works

```
[IRQ 11 fires]
  └─> irq_handler() (top half — atomic context)
        └─> queue_work(own_workqueue, &work)

[System workqueue thread picks it up]
  └─> workqueue_fn() (bottom half — process context)
        └─> pr_info("Executing workqueue function")
```

## Code Notes / Issues

- **`driver12.c` includes `<linux/workqueue.h>` but the `workqueue_fn` is declared before its definition** (line 57-60): `DECLARE_WORK(workqueue, workqueue_fn)` is called before `workqueue_fn` is defined. This is fine in C because `void workqueue_fn(struct work_struct *work);` is forward-declared above it.
- **`driver12.c` vs `driver12Dyn.c`**: Both files are very similar. The key difference is `driver12Dyn.c` creates its own workqueue (`create_workqueue("own_wq")`) instead of using the shared system workqueue. The module description and name could be clearer about which variant is which.
- **Same `dev_id` issue as Driver11**: `(void *)(irq_handler)` used as `dev_id` for shared IRQ — same concern applies.
- **`driver12.c`'s `workqueue_fn` has no real implementation**: It only prints a log. The dynamic version (`driver12Dyn.c`) also only prints. The next driver (Driver13) shows a more useful workqueue function.
