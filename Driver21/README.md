# Driver21 — Kernel Timer (jiffies-based Periodic Timer)

## What This Driver Does

Sets up a periodic kernel timer that fires every 3 seconds using the kernel's standard `timer_list` mechanism. Each time the timer fires, a callback prints a counter value and re-arms itself to fire again.

## Concepts Covered

- `struct timer_list` — the kernel timer structure
- `timer_setup(&timer, callback, flags)` — initialize a timer with a callback (modern API)
- `mod_timer(&timer, jiffies + msecs_to_jiffies(ms))` — set or reset the timer's expiry time
- `timer_delete(&timer)` — cancel and delete the timer (replaces deprecated `del_timer_sync`)
- `jiffies` — the kernel's global tick counter (increments with every system timer interrupt)
- `msecs_to_jiffies(ms)` — converts milliseconds to jiffies (accounts for `HZ` variation)

## Files

| File         | Description |
|--------------|-------------|
| `driver21.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver21.ko
dmesg | tail -5

# Timer fires every 3 seconds
sleep 10
dmesg | tail -10    # Should show ~3 callback entries

sudo rmmod driver21
dmesg | tail -3
```

## How the Periodic Timer Works

```
[Module Load]
  └─> timer_setup() — register callback
  └─> mod_timer() — set first expiry to "now + 3 seconds"

[Timer Fires]
  └─> timer_callback()
        └─> pr_info("Timer function callback here: N")
        └─> mod_timer() — re-arm for "now + 3 seconds" (makes it periodic)

[Module Unload]
  └─> timer_delete() — cancel the timer before unloading
```

## Code Notes / Issues

- **`timer_delete` is the correct modern API**: The comment on line 128 correctly notes that `del_timer` and `del_timer_sync` have been deprecated in newer kernels. This driver already uses the correct `timer_delete`.
- **`add_timer` is commented out** (line 111): `add_timer` and `mod_timer` both start the timer, but `mod_timer` also sets the expiry time. Using `mod_timer` alone (line 114) is correct — `add_timer` would be redundant here.
- **Timer callback runs in softirq context**: No sleeping is allowed in `timer_callback`. The callback here only calls `pr_info` and `mod_timer`, which is safe.
- **`count` is not atomic**: The `count` variable is incremented in the timer callback (softirq context) and could theoretically be read in other contexts. For a simple counter like this it's fine in practice, but `atomic_t` would be more correct.
