# Driver16 — Tasklets (Bottom-Half Interrupt Handling)

## What This Driver Does

Shows how to use **tasklets** — a lightweight form of deferred work that runs in softirq context (not process context like workqueues). When IRQ 11 fires, the interrupt handler schedules a tasklet. The tasklet then runs the work function.

## Tasklets vs Workqueues

| Feature | Tasklet | Workqueue |
|---------|---------|-----------|
| Context | Softirq (atomic) | Process context |
| Can sleep | ❌ No | ✅ Yes |
| Per-CPU execution | Yes (same CPU) | No |
| Use case | Very short, non-sleeping work | Long or sleeping work |

## Concepts Covered

- `DECLARE_TASKLET(name, func)` — statically declare a tasklet (enabled state)
- `tasklet_schedule(&tasklet)` — schedule the tasklet to run
- `tasklet_kill(&tasklet)` — wait for a running tasklet to finish and disable it
- `DECLARE_TASKLET_DISABLED` — declare a tasklet that starts disabled

## Files

| File            | Description |
|-----------------|-------------|
| `driver16.c`    | Module using static tasklet |
| `driver16Dyn.c` | Alternative with dynamic tasklet |
| `Makefile`      | Build configuration |

## Build & Run

```bash
make
sudo insmod driver16.ko
dmesg | tail -5

# Trigger IRQ 11 — tasklet will schedule and run
cat /proc/interrupts | grep etx

sudo rmmod driver16
```

## How It Works

```
[IRQ 11 fires]
  └─> irq_handler() (top half — interrupt context)
        └─> tasklet_schedule(&tasklet)

[Softirq runs at a safe point]
  └─> tasklet_func() (bottom half — softirq context)
        └─> pr_info("Executing tasklet function")
```

## Code Notes / Issues

- **`DECLARE_TASKLET` API changed in kernel 5.9+**: The old signature was `DECLARE_TASKLET(name, func, data)` with a `unsigned long data` parameter. The new signature is `DECLARE_TASKLET(name, func)` with `struct tasklet_struct *` passed to the function. This driver correctly uses the new API. Do not use the old 3-argument form on modern kernels.
- **Tasklets are deprecated in newer kernels**: As of Linux 6.6+, tasklets are being deprecated in favor of threaded IRQs and workqueues. Consider using `request_threaded_irq` for new drivers.
- **Same shared IRQ `dev_id` issue**: Same concern as Driver11 — using the function pointer as `dev_id` is not proper for shared IRQs.
- **`IRQF_SHARED` without device check**: Same concern as Driver11.
