# Driver24 — Exported Symbols (Sharing Between Modules)

## What This Driver Does

Demonstrates how one kernel module can export symbols (functions and variables) so that another module can use them — without modifying the first module's source. This is how the Linux kernel enables modular, plugin-style architecture.

This directory contains **two modules** that must be loaded in order:

| File           | Role | Load Order |
|----------------|------|------------|
| `driver24.c`   | **Exporter** — defines and exports `etx_shared_func()` and `etx_count` | Load first |
| `driver24_2.c` | **Consumer** — uses the exported function and variable from driver24 | Load second |

## Concepts Covered

- `EXPORT_SYMBOL(symbol)` — export a symbol to the global kernel symbol table (available to all modules)
- `EXPORT_SYMBOL_GPL(symbol)` — same, but only available to GPL-licensed modules
- `extern` — declare (not define) a symbol that lives in another module
- `/proc/kallsyms` — list all kernel symbols, including exported ones

## Files

| File            | Description |
|-----------------|-------------|
| `driver24.c`    | Exporter module |
| `driver24_2.c`  | Consumer module |
| `Makefile`      | Builds both modules |

## Build & Run

```bash
make

# Load exporter first
sudo insmod driver24.ko
dmesg | tail -5

# Verify symbols are exported
grep "etx_shared_func\|etx_count" /proc/kallsyms

# Load consumer (depends on exporter being loaded)
sudo insmod driver24_2.ko
dmesg | tail -5

# Trigger the read — calls etx_shared_func from the exporter
cat /dev/etx_Device2

dmesg | tail -5

# Unload in reverse order
sudo rmmod driver24_2
sudo rmmod driver24
```

## How Symbol Sharing Works

```c
// In driver24.c (exporter)
int etx_count = 0;
void etx_shared_func(void) { etx_count++; }
EXPORT_SYMBOL(etx_shared_func);
EXPORT_SYMBOL(etx_count);

// In driver24_2.c (consumer)
extern int etx_count;
void etx_shared_func(void);   // extern by default for functions

// When driver24_2 reads its device:
etx_shared_func();   // calls into driver24's function
pr_info("%d time() shared function called\n", etx_count);
```

## Code Notes / Issues

- **`etx_shared_func` declaration in `driver24_2.c`** (line 27): The comment says "function declaration is extern by default" — this is true in C, but it's still good practice to add the explicit `extern` keyword for clarity.
- **No `MODULE_SOFTDEP` or `modprobe` ordering**: If you try to `insmod driver24_2.ko` before `driver24.ko`, it will fail with an "unknown symbol" error. In a real system, add `MODULE_SOFTDEP("pre: driver24")` in `driver24_2.c` or use `modprobe` with a proper dependency in `modules.dep`.
- **`etx_count` access is not thread-safe**: Both modules access `etx_count` without any locking. Consider using `atomic_t` if concurrent access is possible.
- **`etx_read` returns `1`**: Returning 1 from a read is unusual (tells userspace 1 byte was read). For a proper implementation, either copy data to the user buffer and return the byte count, or return 0 (EOF).
