# Historical Source

These are the sketches as they were salvaged from the team's group chat. Exact duplicates have been removed, and the content is otherwise **unmodified**, including the original comments, formatting and bugs.

| File | Notes |
|---|---|
| `01_speed_basic.ino` | Earliest speed sketch. It has no state tracking, and can divide by zero if `t2 == t1`. |
| `02_speed_state_timeout_buzzer.ino` | Adds the state flags, timeout, buzzer and serial logging |
| `03_speed_lcd_col_row.ino` | `printWithDelay` gains a column argument, and speed is printed as a `String` |
| `04_smart_lighting_ultrasonic.ino` | Separate lighting experiment with 3 ultrasonic sensors |
| `05_integrated_TRUNCATED.ino` | Speed detection plus lighting. **Cut off mid-line and does not compile.** |

The files are kept flat here for reference. To open one in the Arduino IDE, put it in a folder with the same name. Use `../src/` for buildable code. See [../docs/source-provenance.md](../docs/source-provenance.md) for the full analysis.
