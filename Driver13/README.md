# Driver13 — Custom Workqueue + Kernel Linked List

## What This Driver Does

Combines a custom workqueue with the kernel's built-in doubly-linked list. Every time IRQ 11 fires, a work item is queued. The workqueue function allocates a new list node and appends it (with the current `etx_value`) to a linked list. When the device file is read, the driver walks the list and prints all stored nodes.

## Concepts Covered

- `create_workqueue(name)` — create a dedicated kernel workqueue
- `destroy_workqueue(wq)` — destroy the workqueue on cleanup
- `queue_work(wq, &work)` — submit a work item to the custom workqueue
- **Kernel linked list API**: `LIST_HEAD`, `INIT_LIST_HEAD`, `list_add_tail`, `list_for_each_entry`, `list_for_each_entry_safe`, `list_del`, `kfree`
- `struct list_head` — the embedded list node used in Linux kernel data structures

## Files

| File         | Description |
|--------------|-------------|
| `driver13.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver13.ko
dmesg | tail -5

# Each IRQ 11 event adds a node to the list
# Write a value via sysfs (this sets etx_value used in workqueue_fn)
echo 42 | sudo tee /sys/kernel/etx_sysfs/etx_value

# Read the device file to dump all list nodes
cat /dev/etx_device

sudo rmmod driver13
```

## Linked List Node Structure

```c
struct my_list {
    struct list_head list;   // kernel list linkage — must be first or accessible
    int data;                // your actual data
};

LIST_HEAD(Head_Node);        // declare the list head (sentinel node)
```

## Code Notes / Issues

- **`workqueue_fn` reads `etx_value` without locking**: The workqueue function reads the global `etx_value` and the sysfs store also writes it. There is a race condition here. Protect with a spinlock or use an atomic variable.
- **Memory leak if `kmalloc` fails**: In `workqueue_fn`, `temp_node = kmalloc(...)` is not checked for NULL. If `kmalloc` returns NULL, `temp_node->data = etx_value` will cause a NULL pointer dereference (kernel panic). Always check `kmalloc` return value.
- **`sysfs_store` does not update `etx_value`**: The `sscanf` line is **commented out** (line 149): `//sscanf(buf, "%d", &etx_value);`. The sysfs write callback is effectively a no-op. This means `etx_value` is always 0 in the linked list nodes. Uncomment that line if the intent is to store the written value.
- **Same dev_id issue for IRQ**: Same as Driver11/12.
- **`create_workqueue` is deprecated**: Modern kernels prefer `alloc_workqueue("%s", flags, max_active, name)`. `create_workqueue` still works but is a wrapper around `alloc_workqueue`.
