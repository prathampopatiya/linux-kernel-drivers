# Driver22 — High Resolution Timer (hrtimer)

## What This Driver Does

Replaces the standard jiffies-based timer (Driver21) with a **high-resolution timer** (`hrtimer`). The `hrtimer` subsystem provides nanosecond-precision timers, independent of the system's `HZ` tick rate.

## hrtimer vs Standard Timer

| Feature | `timer_list` (Driver21) | `hrtimer` (Driver22) |
|---------|------------------------|----------------------|
| Resolution | HZ-based (typically 1–10ms) | Nanosecond precision |
| API | `timer_setup` + `mod_timer` | `hrtimer_setup` + `hrtimer_start` |
| Restart | `mod_timer` in callback | Return `HRTIMER_RESTART` |
| Clock source | `jiffies` | `ktime_t` (CLOCK_MONOTONIC, etc.) |

## Concepts Covered

- `struct hrtimer` — the high-resolution timer structure
- `hrtimer_setup(&timer, callback, clock, mode)` — initialize the hrtimer (new API)
- `hrtimer_start(&timer, ktime, mode)` — start the timer
- `hrtimer_forward_now(&timer, interval)` — advance the expiry for a periodic effect
- `hrtimer_cancel(&timer)` — stop the timer
- `ktime_set(sec, nsec)` — create a `ktime_t` from seconds + nanoseconds
- `HRTIMER_RESTART` / `HRTIMER_NORESTART` — control whether the timer repeats

## Files

| File         | Description |
|--------------|-------------|
| `driver22.c` | Module source code |
| `Makefile`   | Build configuration |

## Build & Run

```bash
make
sudo insmod driver22.ko
dmesg | tail -5

# Timer fires every ~4 seconds (3s + 1ns in TIMEOUT values)
sleep 15
dmesg | tail -10

sudo rmmod driver22
```

## Timer Interval

```c
#define TIMEOUT_NSEC  ( 1000000000L )   // 1 second in nanoseconds
#define TIMEOUT_SEC   ( 3 )             // 3 seconds

// Total interval = 3 seconds + 1 nanosecond ≈ 4 seconds
ktime = ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC);
```

## Code Notes / Issues

- **`etx_hr_timer.function = &timer_callback` is redundant** (line 130): `hrtimer_setup` already sets the callback (the second argument). The manual assignment on the next line is unnecessary. Remove it.
- **`TIMEOUT_NSEC` adds 1 second on top of `TIMEOUT_SEC`**: The effective interval is **4 seconds** (3 + 1), not 3 seconds. This may be intentional but could be confusing. If a 3-second interval is desired, use `TIMEOUT_NSEC = 0` or restructure the defines.
- **Leftover commented code from Driver21**: Lines 52-59 and 121-126 are commented-out `timer_list` code. These should be removed for a clean codebase.
- **`MODULE_VERSION "1.18"`**: Same version as Driver21. Should be updated to differentiate.
- **`hrtimer_setup` is newer API**: Older kernels used `hrtimer_init` + setting `.function` manually. `hrtimer_setup` is cleaner and available in recent kernels — this is a valid "newer API" usage.
