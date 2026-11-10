# STM32 LED Brake Light (Fade Out)

A car brake-light simulation on STM32: pressing the button turns the LED on instantly, and releasing it fades the LED out smoothly.

## Hardware

- STM32F4 (Discovery)
- LED connected to TIM4, Channel 2 (PWM)
- Button B1 with internal pull-down and EXTI (rising/falling edge)

## Behavior

| Event             | Action                             |
| ----------------- | ---------------------------------- |
| Button pressed    | LED turns on instantly (CCR = ARR) |
| Button held       | LED stays on                       |
| Button released   | Smooth fade-out over ~0.7 s        |
| Press during fade | LED turns on instantly             |

## Implementation

- **PWM:** TIM4, `PSC = 4`, `ARR = 8399` → PWM frequency ≈ 2 kHz, 8400-step resolution
- **State machine:** three states — `OFF` / `ON` / `FADING`
- **Button debounce:** 10 ms time window in `HAL_GPIO_EXTI_Callback`
- **Update rate:** every 1 ms via `HAL_GetTick()` in the main loop (non-blocking)
- **Fade:** linear CCR decrement with `step_fade = 12` per update

## Code structure

led_state (volatile)  ←─ EXTI Callback (only changes state)
        │
        ▼
   led_update()  ←─ every 1 ms from while(1)
        │
        ├── ON      → CCR = ARR
        ├── FADING  → CCR -= step_fade  (until zero)
        └── OFF     → CCR = 0

## Key points

- The CCR register is **not** written inside the ISR — only the state is changed (keeps the ISR short).
- `led_state` is declared `volatile` because it is shared between the ISR and the main loop.
- Non-blocking updates use `HAL_GetTick()` instead of `HAL_Delay()`.
- `int32_t` is used for CCR to avoid wrap-around issues with `uint16_t`.

## Build & Run

STM32CubeIDE project. Open the `.ioc` file and sources, then Build/Run.

## Next steps

- [ ] Replace linear fade with a sine-based fade (`update_sine` is ready)
- [ ] Add adjustable amplitude, frequency, and phase for the sine
- [ ] Replace window-based debounce with confirmation-based (state-based) debounce
  
  


