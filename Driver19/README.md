# Driver19 — Read-Write Spinlock

## What This Driver Does

Uses a **read-write spinlock** to protect a shared variable. Thread 1 is the writer (increments the value) and Thread 2 is a reader (reads the value). Multiple readers can hold the lock simultaneously, but a writer gets exclusive access.

## Why Read-Write Spinlocks?

A regular spinlock is exclusive — only one thread at a time, even for reads. If you have many reader threads and few writers, this wastes CPU. A read-write spinlock allows concurrent reads while ensuring writes are exclusive.

### Key Rules

1. **Multiple readers** can hold the read lock at the same time
2. **Only one writer** can hold the write lock
3. **Writer must wait** for all readers to release before acquiring the lock
4. **New readers are allowed** even while a writer waits — writers can starve!
5. If writer starvation is a concern, use **seqlock** (see Driver26)

## Concepts Covered

- `DEFINE_RWLOCK(name)` — statically declare a read-write spinlock
- `rwlock_t` + `rwlock_init(&lock)` — dynamic initialization
- `read_lock` / `read_unlock` — acquire/release read access
- `write_lock` / `write_unlock` — acquire/release exclusive write access
- `read_lock_irq`, `write_lock_bh` — variants for IRQ/softirq contexts

## Files

| File         | Description |
|--------------|-------------|
| `driver19.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver19.ko
dmesg | tail -5

# Thread1 writes, Thread2 reads — no data races
sleep 5
dmesg | tail -15

sudo rmmod driver19
```

## Code Notes / Issues

- **Same thread error check issue as Driver17/18**: `if(etx_thread1)` should be `if(!IS_ERR(etx_thread1))`.
- **`mutex.h` included but not used**: Same as Driver18 — leftover include.
- **`MODULE_DESCRIPTION` is copy-pasted**: Still says "Simple device Driver allocating memory..." — should describe read-write spinlocks.
- **MODULE_VERSION is `"1.16"`**: Same as Driver18. This appears to be a copy-paste error — should be a unique version for Driver19.
- **Writer starvation is possible**: This driver does not demonstrate or mention that readers can starve writers in a read-write spinlock. Consider seqlock (Driver26) for write-priority scenarios.
- **`DEFINE_RWLOCK` makes it static**: The `static` keyword was used in the definition, which is correct.
