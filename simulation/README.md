# Simulation

The team designed the circuit in **Tinkercad Circuits**. The project presentation includes two images of that design:

- [Breadboard / circuit view](../images/tinkercad_circuit.png)
- [Auto-generated schematic](../images/tinkercad_schematic.png), dated 11 March 2025

The schematic shows these parts:

- Arduino Uno
- 16×2 LCD with its SDA and SCL wired to A4 and A5
- Two 3-pin sensor modules (VCC/OUT/GND)
- Two red LEDs
- An N-channel MOSFET with a 220 Ω resistor
- A photoresistor
- A piezo buzzer

That set of parts matches the **integrated** sketch (`src/integrated_system`).

**This repository does not contain the Tinkercad project itself**, only the screenshots above. It hasn't been confirmed that the salvaged code was the code run in the simulation, and it's also unknown whether the simulated sensor parts were the same type as the physical IR modules.

If the Tinkercad project is still available, add its public share link here.
