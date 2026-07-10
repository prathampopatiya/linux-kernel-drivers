# Driver11 — Interrupt Handling (Shared IRQ + Sysfs)

## What This Driver Does

Extends the sysfs driver (Driver10) by registering an interrupt handler for IRQ line 11. When the interrupt fires, the handler prints a log message. This is a foundation for understanding how real hardware interrupts are handled in a kernel driver.

> **Note**: IRQ 11 is shared on x86 systems. The handler uses `IRQF_SHARED`, meaning it can coexist with other drivers sharing the same IRQ line. The handler must check if the interrupt is actually from its device (in real drivers); this example skips that for simplicity.

## Concepts Covered

- `request_irq(irq, handler, flags, name, dev_id)` — register an interrupt service routine (ISR)
- `free_irq(irq, dev_id)` — deregister the interrupt handler
- `IRQF_SHARED` — allows multiple drivers to share the same IRQ line
- `irqreturn_t` return values: `IRQ_HANDLED` (we handled it) vs `IRQ_NONE` (not ours)
- The interrupt handler runs in atomic context — no sleeping is allowed

## Files

| File         | Description |
|--------------|-------------|
| `driver11.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver11.ko
dmesg | tail -5

# Trigger a software interrupt to test (x86 only — for testing purposes)
# The handler will fire naturally when IRQ 11 is triggered by hardware

# Check registered interrupts
cat /proc/interrupts | grep etx

sudo rmmod driver11
```

## Interrupt Handler

```c
static irqreturn_t irq_handler(int irq, void *dev_id) {
    pr_info("Shared IRQ: interrupt Occurred\n");
    return IRQ_HANDLED;
}
```

## Code Notes / Issues

- **`dev_id` is set to `(void *)(irq_handler)`** — using the function pointer as the device ID is a common pattern in these tutorial drivers, but in production code, `dev_id` should point to your private device structure so the handler can distinguish which device triggered the interrupt.
- **No check that IRQ actually belongs to this device**: With `IRQF_SHARED`, multiple devices share IRQ 11. A real driver must check a hardware status register to confirm the interrupt came from its device. Without this check, the handler claims every IRQ 11 as handled (`IRQ_HANDLED`), which can prevent other devices from receiving their interrupts.
- **IRQ 11 may not be available**: On some systems, IRQ 11 may not exist or may not be shareable. `request_irq` will return an error in that case. Check `dmesg` for messages from the driver init.
- **`volatile int etx_value`**: Same issue as Driver10 — prefer atomics or proper locking over `volatile`.
