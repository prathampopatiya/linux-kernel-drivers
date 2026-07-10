# Driver14 — Linked List with Workqueue + Software Interrupt Trigger

## What This Driver Does

This driver is similar to Driver13 (workqueue + linked list), but adds a key feature: the `etx_write` function **triggers a software interrupt** (`int $0x3b` = IRQ 59) which fires the IRQ handler, which queues the work item, which adds a node to the linked list. Reading the device file then dumps the entire list.

This demonstrates a full cycle: userspace write → software interrupt → workqueue → data structure update → device read shows result.

## Concepts Covered

- Using inline assembly `asm("int $0x3b")` to generate a software interrupt
- All concepts from Driver13 (workqueue, linked list, IRQ handler)
- How `etx_write` serves as the trigger instead of a real hardware event

## Files

| File         | Description |
|--------------|-------------|
| `driver14.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver14.ko
dmesg | tail -5

# Write to the device — this triggers int $0x3b → workqueue → list node added
echo "5" | sudo tee /dev/etx_device

# Read to see the list
cat /dev/etx_device

# Write multiple times to accumulate nodes
echo "10" | sudo tee /dev/etx_device
echo "20" | sudo tee /dev/etx_device
cat /dev/etx_device

sudo rmmod driver14
```

## How the Trigger Works

```
echo "5" > /dev/etx_device
  └─> etx_write()
        └─> sscanf(buf, "%d", &etx_value)   -- parses "5"
        └─> asm("int $0x3b")                 -- fires software interrupt 59 (IRQ 11 equivalent)
              └─> irq_handler()
                    └─> queue_work(own_workqueue, &work)
                          └─> workqueue_fn()
                                └─> kmalloc new node, node->data = etx_value, list_add_tail
```

## Code Notes / Issues

- **`asm("int $0x3b")` is x86-only**: This assembly instruction is not portable. It will not work on ARM, RISC-V, or other architectures. For a real driver, this would be a hardware interrupt, not a software-generated one.
- **`int $0x3b` = 0x3b = 59 decimal**: This is IRQ 59, not IRQ 11. The `IRQ_NO` macro is `11`. There is a mismatch — the software interrupt triggers IRQ 59 but the driver registers for IRQ 11. The interrupt handler will not be called this way unless IRQ 11 happens to be on vector 0x3b on your system. This is a known quirk of the tutorial approach.
- **`kmalloc` not checked for NULL**: Same issue as Driver13.
- **`sysfs_store` still has the `sscanf` commented out** (line 149): Same issue as Driver13 — the sysfs write is a no-op, but `etx_write` does parse the value correctly.
- **Cleanup order in `etx_driver_exit`**: The workqueue is destroyed before freeing the linked list nodes. This is correct — ensure no more work items are queued before walking the list.
