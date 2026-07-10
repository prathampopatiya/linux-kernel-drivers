# Driver-7 — IOCTL (Input/Output Control)

## What This Driver Does

Implements `ioctl` support in a character device driver. IOCTL lets a userspace program send custom commands to the kernel driver — commands that don't fit the normal `read`/`write` model. This driver supports writing an integer value into the kernel and reading it back.

## Concepts Covered

- `_IOW(type, nr, datatype)` — define an IOCTL command that **W**rites data from user → kernel
- `_IOR(type, nr, datatype)` — define an IOCTL command that **R**eads data from kernel → user
- `unlocked_ioctl` in `file_operations` — the modern (non-BKL) ioctl handler
- `copy_from_user` / `copy_to_user` inside the ioctl handler
- How IOCTL command numbers are constructed (magic number, sequence number, direction, size)

## Files

| File        | Description |
|-------------|-------------|
| `driver7.c` | Kernel module source |
| `test.c`    | Userspace test program |
| `Makefile`  | Build configuration |

## IOCTL Commands Defined

```c
#define WR_VALUE  _IOW('a', 'a', int32_t*)   // Write a value into the driver
#define RD_VALUE  _IOR('a', 'b', int32_t*)   // Read the value back from the driver
```

## Build & Run

```bash
make

sudo insmod driver7.ko
dmesg | tail -5

# Compile and run the test app
gcc -o test test.c
sudo ./test        # Uses ioctl() to write and read a value

dmesg | tail -10
sudo rmmod driver7
```

## How IOCTL Works (Flow)

```
userspace app
    |
    |-- ioctl(fd, WR_VALUE, &value)   -- sends value to kernel
    |                                    driver stores it in global 'value'
    |
    |-- ioctl(fd, RD_VALUE, &result)  -- kernel copies stored value back to user
```

## Code Notes / Issues

- **IOCTL command encoding**: `_IOW('a','a',int32_t*)` — using a `char` (`'a'`) as the `nr` field is unusual but works. Conventionally, numbers like `0`, `1`, `2` are used for the `nr` field.
- **No range validation on `arg`**: The driver does not validate that `arg` is a valid user-space pointer before passing it to `copy_from_user`. While `copy_from_user` handles NULL gracefully, additional validation improves robustness.
- **`value` is a global**: A global `int32_t value` is shared across all file descriptor opens. In a real driver this would need synchronization (mutex/spinlock) if multiple processes could use the device simultaneously.
- **Typo**: Line 81 — `"successfull"` should be `"successful"`.
