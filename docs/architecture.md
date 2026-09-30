# Architecture

## Execution model

Each sketch is one Arduino program with a single-threaded super-loop:

- `setup()` runs once. It initialises the LCD over I2C, sets pin modes, opens serial at 9600 baud and shows a splash screen.
- `loop()` runs repeatedly. It reads the sensors with `digitalRead` or `analogRead` and acts on them straight away.

The code has no interrupts, timers, RTOS or background tasks. All waiting is done with blocking `delay()` calls, so while a delay runs nothing else happens.

## Integrated system: one loop pass

```
┌───────────────────────── loop() ──────────────────────────┐
│ 1. updateStreetLights()                                   │
│      ldr = analogRead(A0)                                 │
│      if ldr < 200:                (dark)                  │
│          LED1, LED2 ← PWM 30;  MOSFET ← HIGH              │
│          if IR1 == LOW or IR2 == LOW:                     │
│              LED1, LED2 ← HIGH (full)                     │
│      else:                        (light)                 │
│          LED1, LED2, MOSFET ← LOW                         │
│                                                           │
│ 2. Speed state machine                                    │
│      IDLE    --IR1 LOW-->             WAITING (t1 = now)  │
│      WAITING --IR2 LOW-->             RESULT  (t2 = now)  │
│      WAITING --now - t1 > 5000 ms-->  TIMEOUT             │
│      RESULT / TIMEOUT --after display delay--> IDLE       │
└───────────────────────────────────────────────────────────┘
```

### Result handling (RESULT state)

```
LCD "Object detected"                     delay 1000 ms
v = (0.2 / ((t2 - t1) / 1000.0)) * 3.6    km/h
v > 5.0 ?
  yes → buzzer ON, LCD "Overspeed detected!!" + speed, delay 3000 ms, buzzer OFF
  no  → LCD "Normal Speed." + speed
serial log
delay 3000 ms
resetMeasurement()  → flags cleared, buzzer OFF, LCD "Ready..."
```

The result display takes about 4 s for a normal speed and about 7 s for an overspeed. During that time `updateStreetLights()` doesn't run, so the LEDs keep whatever state they were set to at the start of that pass. IR2 was LOW on that pass, so at night that means full brightness. This is a side effect of the blocking design rather than a deliberate hold feature.

## Sensor usage

| Sensor | Read with | Interpreted as |
|---|---|---|
| IR obstacle module | `digitalRead` | `LOW` means an object is present |
| LDR | `analogRead` (0–1023) | Integrated: `< 200` is dark. Ultrasonic sketch: `> 800` is bright. |
| Ultrasonic (ultrasonic sketch only) | 10 µs TRIG pulse, `pulseIn(ECHO, HIGH)` | `duration * 0.034 / 2` cm; `≤ 20` cm means an object is present |

## Outputs

| Output | Driven with | Behaviour |
|---|---|---|
| LEDs | `analogWrite` (dim) and `digitalWrite` (full or off) | Street-light brightness |
| MOSFET gate | `digitalWrite` | On when dark, off when light |
| Buzzer | `digitalWrite` | On for 3 s when overspeed is detected |
| 16×2 LCD | `LiquidCrystal_I2C` over I2C | Status and speed text |
| Serial | `Serial.println` | Debug log: LDR value, timestamps, speed |

## How the sketches relate

```
01_speed_basic ──► 02_speed_state_timeout_buzzer ──► 03_speed_lcd_col_row ──┐
                                                                            ├─► 05_integrated
                         (lighting logic, IR-based, 2 LEDs + MOSFET) ───────┘

04_smart_lighting_ultrasonic   separate experiment (3 ultrasonic sensors, 3 LEDs);
                               pins clash with the speed sketches, not merged
```

The version order is inferred from how the code changes between versions. See [source-provenance.md](source-provenance.md).
