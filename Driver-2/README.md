# Driver-2 — Module Parameters

## What This Driver Does

Demonstrates how to pass parameters into a kernel module at load time using `module_param` and `module_param_array`. It also shows how to register a callback that fires whenever a specific parameter is changed from userspace (via `/sys/module/`).

## Concepts Covered

- `module_param(name, type, permission)` — expose a variable as a module parameter
- `module_param_array(name, type, nump, permission)` — expose an array as a parameter
- `module_param_cb` — register a callback triggered when a parameter value changes
- `struct kernel_param_ops` — custom get/set operations for a parameter
- `param_set_int` / `param_get_int` — built-in helpers for integer parameters
- Sysfs permissions (`S_IRUSR`, `S_IWUSR`, `S_IRUGO`) control who can read/write the parameter files

## Files

| File        | Description |
|-------------|-------------|
| `driver2.c` | Module source code |
| `Makefile`  | Build configuration |

## Build & Run

```bash
make

# Load with parameters
sudo insmod driver2.ko valueEtx=10 nameEtx="hello" arr_valueEtx=1,2,3,4 cb_valueEtx=5

# Check initial values in the log
dmesg | tail -10

# Change the callback parameter at runtime
echo 99 | sudo tee /sys/module/driver2/parameters/cb_valueEtx

# Watch the callback fire
dmesg | tail -5

sudo rmmod driver2
```

## Parameter Files (after loading)

| Path | Description |
|------|-------------|
| `/sys/module/driver2/parameters/valueEtx`     | Integer parameter |
| `/sys/module/driver2/parameters/nameEtx`      | String parameter |
| `/sys/module/driver2/parameters/arr_valueEtx` | Integer array parameter |
| `/sys/module/driver2/parameters/cb_valueEtx`  | Callback-enabled integer |

## Code Notes / Issues

- **`nameEtx` default value**: The `nameEtx` pointer is declared but not initialized to a default string. If the module is loaded without the `nameEtx=` argument, `pr_info("nameEtx = %s\n", nameEtx)` will dereference a NULL pointer and likely cause a kernel oops. Add a default: `char *nameEtx = "default";`.
- The `notify_param` callback returns `-1` on error. The convention in the kernel is to return a negative `errno` value, e.g. `-EINVAL`.
