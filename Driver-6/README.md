# Driver-6 — Kernel Memory & Data Transfer (kmalloc + copy_to/from_user)

## What This Driver Does

Allocates a 1 KB kernel buffer using `kmalloc`, pre-fills it with `"hello universe!!"`, and lets a userspace program read from and write to the device file. Data is transferred safely between kernel and userspace using `copy_to_user` / `copy_from_user`.

## Concepts Covered

- `kmalloc(size, flags)` — allocate kernel memory; `GFP_KERNEL` is the standard flag for sleepable contexts
- `kfree(ptr)` — release allocated kernel memory
- `copy_to_user(dst, src, len)` — safely copy data from kernel buffer to a user buffer
- `copy_from_user(dst, src, len)` — safely copy data from a user buffer into the kernel
- Why direct pointer access between kernel and user memory is not safe (different address spaces)

## Files

| File        | Description |
|-------------|-------------|
| `driver6.c` | Module source code |
| `test.c`    | Userspace test program |
| `Makefile`  | Build configuration |

## Build & Run

```bash
make

# Load the driver
sudo insmod driver6.ko
dmesg | tail -5

# Read the pre-filled buffer
cat /dev/etx_Devicee      # Should print "hello universe!!"

# Write new data
echo "new data" > /dev/etx_Devicee

# Read it back
cat /dev/etx_Devicee

sudo rmmod driver6
```

## Compiling and Running the Test Program

```bash
gcc -o test test.c
sudo ./test
```

## Code Notes / Issues

- **`etx_read` always returns `MEMSIZE` (1024)**: Even if the user buffer is smaller than 1024 bytes, this driver copies the full 1024. This can overflow the user buffer. The `len` argument should be respected: use `min(len, MEMSIZE)`.
- **`copy_to_user` error is silently ignored**: The `if(copy_to_user(...))` block prints an error but still returns `MEMSIZE`. On failure, the function should return `-EFAULT`.
- **`copy_from_user` ignores `len` bounds**: Writing more than 1024 bytes could overflow `kernel_buffer`. The write function should clamp `len` to `MEMSIZE`.
- **`kmalloc` zero-check**: Line 85 checks `== 0` instead of `== NULL`. Both work in C, but `== NULL` is more idiomatic and clearer.
- **Error path for `kmalloc` failure**: The label `r_device` is used, but `class_destroy` is not called in that path (only `device_destroy`). The label should unwind more carefully.
- **Typo in log messages**: `"Data Read: Donee"` and `"Device Write: Donee"` — "Donee" should be "Done".
