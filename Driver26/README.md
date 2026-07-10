# Driver26 — Seqlock (Writer-Priority Read-Write Lock)

## What This Driver Does

Implements **seqlock** — a read-write lock that gives **priority to the writer**. Readers do not block writers; instead, they detect if a write occurred during their read and retry. This makes seqlocks ideal for data that is read frequently but written rarely, and where writer starvation (as in rwlock) is unacceptable.

## Seqlock vs Read-Write Spinlock

| | Read-Write Spinlock | Seqlock |
|---|---|---|
| Priority | Reader-biased | Writer-biased |
| Writer blocked by readers | Yes | No |
| Reader blocked by writer | Yes | No (retries instead) |
| Reader overhead | Low | Slightly higher (retry logic) |
| Best for | Mostly-read data, short critical sections | Data updated by writer that readers can tolerate retrying |

## Concepts Covered

- `seqlock_t` — the seqlock data structure (combines a spinlock + sequence counter)
- `seqlock_init(&lock)` — initialize the seqlock
- `write_seqlock(&lock)` / `write_sequnlock(&lock)` — exclusive write access (increments sequence counter)
- `read_seqbegin(&lock)` — returns the current sequence number before reading
- `read_seqretry(&lock, seq)` — returns true if a write occurred since `read_seqbegin`; reader must retry

## Files

| File         | Description |
|--------------|-------------|
| `driver26.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver26.ko
dmesg | tail -5

# Thread1 writes, Thread2 reads with retry logic
sleep 5
dmesg | tail -15

sudo rmmod driver26
```

## Reader Pattern (Retry Loop)

```c
unsigned int seq_no;
unsigned long read_value;

do {
    seq_no = read_seqbegin(&etx_seq_lock);   // take sequence snapshot
    read_value = etx_global_variable;         // read (may be inconsistent)
} while (read_seqretry(&etx_seq_lock, seq_no)); // retry if write happened

// read_value is now consistent
```

## Code Notes / Issues

- **`write_sequnlock` typo**: Line 47 uses `write_sequnlock(&etx_seq_lock)` — this is actually the correct kernel function name (`sequnlock`, not `sequnlock`). It looks like a typo but is intentional in the Linux kernel API. Just be aware when searching documentation.
- **Same thread error check issue** (lines 134, 142): `if(etx_thread1)` should be `if(!IS_ERR(etx_thread1))` since `kthread_run` returns `ERR_PTR`, not NULL on failure.
- **Same `pr_err` for success** (lines 135, 143): Success messages should use `pr_info`, not `pr_err`.
- **`seqlock_init` called after thread creation** (line 150): The threads are started before `seqlock_init`. If Thread1 calls `write_seqlock` before the seqlock is initialized, it may panic. Move `seqlock_init` to **before** the thread creation calls.
- **Device node name typo**: `"etc_seqlock_device"` — "etc" should be "etx".
