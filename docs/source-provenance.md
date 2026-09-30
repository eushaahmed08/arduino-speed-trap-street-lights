# Source Provenance

This document records how the repository was put together from the salvaged material, and how confident each claim is.

## Source material

1. **Arduino code blocks** copied out of the team's group chat. There are 10 blocks, some of them exact duplicates, and some truncated.
2. **The final project presentation** (17 slides). It contains the title, team, objectives, a flowchart, Tinkercad screenshots and a component/budget table.

## Salvaged code blocks

| # | Description | Kept as | Status |
|---|---|---|---|
| 1 | Minimal speed sketch: LCD at 0x27, IR active HIGH, speed computed every loop | `historical/01_speed_basic.ino` | Complete |
| 2 | Speed sketch with state flags, timeout, buzzer, serial logging; `printWithDelay(text,row)` | `historical/02_speed_state_timeout_buzzer.ino` | Complete |
| 3 | Exact duplicate of #2 | not kept | Duplicate |
| 4 | Same as #2 but `printWithDelay(text,col,row)` and speed printed as a 2-decimal `String` | `historical/03_speed_lcd_col_row.ino` | Complete |
| 5 | Exact duplicate of #4 | not kept | Duplicate |
| 6 | Lighting: 3 ultrasonic sensors, LDR, 3 PWM LEDs | `historical/04_smart_lighting_ultrasonic.ino` | Complete |
| 7 | Integrated: #4 plus the LDR/IR/MOSFET lighting block (preceded in the chat by the words "eta code", Bengali for "this is the code") | `historical/05_integrated_TRUNCATED.ino` | **Truncated** |
| 8, 9 | Exact duplicates of #7, truncated at the same point | not kept | Duplicate |

The duplicates were confirmed with a line-by-line `diff`.

## What was changed in `src/`

These changes apply to all three cleaned sketches:

- Constants renamed for clarity (`DISTANCE` → `SENSOR_DISTANCE_M`, `TIMEOUT` → `TIMEOUT_MS`, `SPEED_THRESHOLD` → `SPEED_THRESHOLD_KMH`), and magic numbers in the lighting code named.
- Misleading comments corrected. For example, "LED1 connected to pin 9" was on a line defining pin 10, and "set the LCD address to 0x27" was on a line using 0x3F.
- The duplicated LCD result code was merged into `showSpeedOnLcd()`. The output text, cursor positions and timing are unchanged.
- The inline lighting block was moved into `updateStreetLights()`. Its logic is unchanged.
- `volatile` was removed from `t1`/`t2`. No interrupt touches them, so it had no effect.
- The unused `#include <string.h>` was removed.
- Helper functions are defined before use, so the sketches also compile outside the Arduino IDE's automatic prototype generation.

**Kept on purpose, even though they are flaws:** blocking delays, the LCD column overlap and 20-character headline, the `pulseIn` timeout being read as 0 cm, and the different LDR thresholds in the two lighting versions. These are recorded as limitations in the README rather than silently fixed.

**Not carried into `src/`:** version #1 (`01_speed_basic`). It has been superseded, and it can divide by zero when `t2 == t1`. It is kept only in `historical/`.

## Reconstruction of the truncated integrated sketch

The salvaged copy ends in the middle of this line:

```cpp
Serial.println("Speed: " + String(
```

Everything before that point matches `03_speed_lcd_col_row.ino` exactly in the speed-detection code, apart from whitespace. So the remainder of `src/integrated_system/integrated_system.ino` (the end of that line, the closing separator, the reset delay, the timeout branch and `resetMeasurement()`) was taken from that sketch and is marked `RECONSTRUCTED` in the code. This is the most likely ending, but it can't be proven: the final version may have changed something after the cut-off point.

## A. Confirmed from source

- The project title, the four team members, and that it was a university group project (from the presentation)
- An Arduino-based implementation in C++ (Arduino sketches)
- Arduino Uno as the board (presentation BOM)
- Two IR sensor modules on D8 and D9, read active-LOW in all versions except the first. A code comment names them "MH Flying Fish".
- A 16×2 I2C LCD driven by the `LiquidCrystal_I2C` library, at address 0x27 in the earliest version and 0x3F later
- A buzzer on D13
- The speed formula `v = (0.2 m / Δt s) × 3.6`, with Δt from `millis()` timestamps
- The overspeed threshold of 5.0 km/h, the 5 s timeout, and the LCD and serial messages
- The integrated lighting logic: LDR on A0, dark below 200, LEDs on D10/D11 at PWM 30, full brightness when an IR sensor detects something, MOSFET gate on D6 switched on/off
- The ultrasonic lighting variant: 3 sensors on D2–D7, LEDs on D9–D11, off above an LDR reading of 800, 20% (51) default, 255 for 5 s when an object is within 20 cm
- Serial output at 9600 baud
- Tinkercad was used for the circuit design, and the schematic is dated 11 March 2025 (from the presentation)
- The BOM lists a solar panel, battery, charge controller, step-up converter, diode, MOSFET, 2 LEDs, 3 resistors and a total estimated cost of 2580 BDT

## B. Reasonable inference

- **Version order.** The order 01 → 02 → 03 → 05 follows from the progressive changes: added state flags, then the changed `printWithDelay` signature, then the added lighting block.
- **The integrated sketch is the final demo version.** Its components (2 LEDs, MOSFET, LDR, 2 IR sensors, LCD, buzzer) match the Tinkercad schematic and the BOM, and "eta code" suggests it was shared as the code to use.
- **The ending of the integrated sketch** is as reconstructed above.
- **The ultrasonic sketch was an earlier or alternative experiment.** Ultrasonic sensors aren't in the BOM or the Tinkercad schematic, and it uses pin 9, which clashes with IR2.
- **The MOSFET probably switched a separate light or LED string** from the solar/battery supply. The code only shows a gate on D6, and the load isn't named.
- **The LDR divider was wired differently in the two lighting builds**, given their opposite threshold directions.
- **The LCD is on A4/A5**, the Uno I2C defaults. The Tinkercad schematic appears to show this.

## C. Cannot be established

- Whether the reconstructed integrated sketch is exactly what ran in the final demonstration
- Whether the physical prototype worked as intended, and how accurate the speed readings were. No test data, measurements or results exist in the material.
- Which team member wrote which code, wired which hardware, or made which design decisions
- The exact part numbers of the ultrasonic sensors, LDR, LEDs and MOSFET
- What load the MOSFET drove, and how the solar/charging hardware was wired
- Whether the Tinkercad simulation used the same sensor types as the physical build. The schematic shows generic 3-pin sensor modules.
- Any traffic-signal (red/amber/green) implementation. None appears in the salvaged code.
- Whether the solar, battery and charging hardware was actually built into the prototype or only budgeted
