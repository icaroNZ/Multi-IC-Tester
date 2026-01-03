# Multi-IC Tester PCB - Comprehensive Pinout Mapping

## Overview

This PCB design supports testing three different ICs using a single Arduino Mega 2560:
- **Z80 CPU** (40-pin DIP)
- **6502 CPU** (40-pin DIP)
- **HM62256B SRAM** (28-pin DIP)

**Testing Mode:** ONE IC at a time (other sockets empty)
**Control:** UART command selects which IC to test
**Strategy:** Maximum pin sharing and reuse

---

## Quick Reference Tables

### IC Socket Locations on PCB

```
┌─────────────────────────────────────┐
│                                     │
│    ┌────────┐  ┌────────┐          │
│    │  6502  │  │  Z80   │          │
│    │ 40-pin │  │ 40-pin │          │
│    └────────┘  └────────┘          │
│                                     │
│         ┌────────┐                 │
│         │ 62256  │                 │
│         │ 28-pin │                 │
│         └────────┘                 │
│                                     │
│        Arduino Mega 2560            │
└─────────────────────────────────────┘
```

---

## Complete Pin Mapping Table

### Address Bus (A0-A15)

**Shared across all ICs** - Arduino drives address bus for all testing modes.

| Address | Arduino Mega | Z80 Pin | 6502 Pin | 62256 Pin | Notes |
|---------|--------------|---------|----------|-----------|-------|
| **A0** | **PA0 (22)** | 30 | 9 | 10 | |
| **A1** | **PA1 (23)** | 31 | 10 | 9 | |
| **A2** | **PA2 (24)** | 32 | 11 | 8 | |
| **A3** | **PA3 (25)** | 33 | 12 | 7 | |
| **A4** | **PA4 (26)** | 34 | 13 | 6 | |
| **A5** | **PA5 (27)** | 35 | 14 | 5 | |
| **A6** | **PA6 (28)** | 36 | 15 | 4 | |
| **A7** | **PA7 (29)** | 37 | 16 | 3 | |
| **A8** | **PC7 (30)** | 38 | 17 | 25 | |
| **A9** | **PC6 (31)** | 39 | 18 | 24 | |
| **A10** | **PC5 (32)** | 40 | 19 | 21 | |
| **A11** | **PC4 (33)** | 1 | 20 | 23 | Z80: wraps to pin 1 |
| **A12** | **PC3 (34)** | 2 | 22 | 2 | |
| **A13** | **PC2 (35)** | 3 | 23 | 26 | |
| **A14** | **PC1 (36)** | 4 | 24 | 1 | |
| **A15** | **PC0 (37)** | 5 | 25 | — | Not used for 62256 |

**Port Configuration:**
- PORTA (A0-A7): Arduino pins 22-29 → OUTPUT mode
- PORTC (A8-A15): Arduino pins 30-37 → OUTPUT mode

---

### Data Bus (D0-D7)

**Shared across all ICs** - Bidirectional, direction controlled by firmware.

| Data | Arduino Mega | Z80 Pin | 6502 Pin | 62256 Pin | Notes |
|------|--------------|---------|----------|-----------|-------|
| **D0** | **PL0 (49)** | 14 | 33 | 11 (I/O0) | LSB |
| **D1** | **PL1 (48)** | 15 | 32 | 12 (I/O1) | |
| **D2** | **PL2 (47)** | 12 | 31 | 13 (I/O2) | |
| **D3** | **PL3 (46)** | 8 | 30 | 15 (I/O3) | |
| **D4** | **PL4 (45)** | 7 | 29 | 16 (I/O4) | |
| **D5** | **PL5 (44)** | 9 | 28 | 17 (I/O5) | |
| **D6** | **PL6 (43)** | 10 | 27 | 18 (I/O6) | |
| **D7** | **PL7 (42)** | 13 | 26 | 19 (I/O7) | MSB |

**Port Configuration:**
- PORTL (D0-D7): Arduino pins 42-49 → Bidirectional (DDR controlled by firmware)
  - **INPUT** (0x00): When reading from IC
  - **OUTPUT** (0xFF): When writing to IC

---

### Clock Signals

| Signal | Arduino Mega | Z80 Pin | 6502 Pin | 62256 Pin | Direction | Notes |
|--------|--------------|---------|----------|-----------|-----------|-------|
| **CLK** | **PE3 (5)** | 6 | 37 (Φ0) | — | OUT | Timer3 generates clock for both CPUs |
| **Φ1 Monitor** | **PD0 (21)** | — | 3 (Φ1) | — | IN | Read 6502 phase 1 output |
| **Φ2 Monitor** | **PD1 (20)** | — | 39 (Φ2) | — | IN | Read 6502 phase 2 output |

**Configuration:**
- **Z80 Mode:** PE3 (Timer3) → Z80 pin 6 (CLK)
- **6502 Mode:** PE3 (Timer3) → 6502 pin 37 (Φ0 input)
  - Monitor outputs: PD0 ← 6502 pin 3 (Φ1), PD1 ← 6502 pin 39 (Φ2)
- **62256 Mode:** No clock needed

---

### Reset Signal

**Shared reset - Only one IC active at a time.**

| Signal | Arduino Mega | Z80 Pin | 6502 Pin | 62256 Pin | Direction | Notes |
|--------|--------------|---------|----------|-----------|-----------|-------|
| **/RESET** | **PH6 (9)** | 26 | 40 (RES) | — | OUT | Active LOW reset for CPUs |

**Usage:**
- **Z80:** /RESET (active LOW, hold 3+ cycles)
- **6502:** RES (active LOW, hold 2+ cycles)
- **62256:** Not connected (chip select used instead)

---

### Control Signals - Read/Write Operations

These signals have different meanings for each IC but share Arduino pins through firmware control.

| Arduino Pin | Direction | Z80 Signal | 6502 Signal | 62256 Signal | Notes |
|-------------|-----------|------------|-------------|--------------|-------|
| **PG0 (41)** | IN | /MREQ (27) | — | — | Memory request (Z80 only) |
| **PG1 (40)** | IN | /IORQ (20) | — | — | I/O request (Z80 only) |
| **PG2 (39)** | Bi | /RD (21) | R/W (34) | /OE (22) | Read signal / R-W control / Output Enable |
| **PG3 (38)** | Bi | /WR (22) | — | /WE (27) | Write signal / Not used / Write Enable |

**PG2 (Pin 39) - Read Control - CRITICAL:**
- **Z80 Mode:** INPUT - Monitor /RD (active LOW when Z80 reads)
- **6502 Mode:** INPUT - Monitor R/W (HIGH=read, LOW=write) - **INVERTED LOGIC**
- **62256 Mode:** OUTPUT - Control /OE (drive LOW to read from SRAM)

**PG3 (Pin 38) - Write Control:**
- **Z80 Mode:** INPUT - Monitor /WR (active LOW when Z80 writes)
- **6502 Mode:** Not connected (R/W pin handles both)
- **62256 Mode:** OUTPUT - Control /WE (drive LOW to write to SRAM)

**PG0 (Pin 41) - Chip Select / Memory Request:**
- **Z80 Mode:** INPUT - Monitor /MREQ (active LOW for memory cycles)
- **6502 Mode:** Not used
- **62256 Mode:** OUTPUT - Control /CS (drive LOW to select chip)

---

### Control Signals - Wait/Ready

| Arduino Pin | Direction | Z80 Signal | 6502 Signal | 62256 Signal | Notes |
|-------------|-----------|------------|-------------|--------------|-------|
| **PB4 (10)** | OUT | /WAIT (24) | RDY (2) | — | Active LOW wait (Z80) / Active HIGH ready (6502) |

**Configuration:**
- **Z80 Mode:** OUTPUT - Drive /WAIT LOW to insert wait states
- **6502 Mode:** OUTPUT - Drive RDY HIGH for normal operation, LOW to pause
- **62256 Mode:** Not used

---

### Control Signals - Interrupts

| Arduino Pin | Direction | Z80 Signal | 6502 Signal | 62256 Signal | Notes |
|-------------|-----------|------------|-------------|--------------|-------|
| **PB5 (11)** | OUT | /INT (16) | IRQ (4) | — | Maskable interrupt (active LOW) |
| **PB6 (12)** | OUT | /NMI (17) | NMI (6) | — | Non-maskable interrupt (active LOW) |

**Configuration:**
- **Z80 Mode:** OUTPUT - Drive LOW to trigger interrupt
- **6502 Mode:** OUTPUT - Drive LOW to trigger interrupt
- **62256 Mode:** Not used

---

### Control Signals - Status/Sync

| Arduino Pin | Direction | Z80 Signal | 6502 Signal | 62256 Signal | Notes |
|-------------|-----------|------------|-------------|--------------|-------|
| **PH3 (6)** | IN | /M1 (27) | SYNC (7) | — | Opcode fetch (Z80) / Sync output (6502) |
| **PE4 (2)** | IN | /HALT (18) | — | — | Halt status (Z80 only) |

**PH3 (Pin 6) - CRITICAL - Direction changes:**
- **Z80 Mode:** INPUT - Monitor /M1 (active LOW during opcode fetch)
- **6502 Mode:** INPUT - Monitor SYNC (active HIGH during opcode fetch) - **INVERTED LOGIC**
- **62256 Mode:** Not used

---

### Control Signals - Z80 Specific (Not used for 6502/62256)

| Arduino Pin | Direction | Z80 Signal | Purpose | 6502/62256 |
|-------------|-----------|------------|---------|------------|
| **PH4 (7)** | IN | /RFSH (28) | DRAM refresh cycle | Not connected |
| **PH5 (8)** | IN | /BUSACK (23) | Bus acknowledge | Not connected |
| **PB7 (13)** | OUT | /BUSREQ (25) | Bus request (DMA) | Not connected |

---

### Control Signals - 6502 Specific (New connections)

| Arduino Pin | Direction | 6502 Signal | Purpose | Z80/62256 |
|-------------|-----------|-------------|---------|-----------|
| **PD3 (18)** | OUT | S.O. (38) | Set Overflow flag | Not connected |

**S.O. (Set Overflow):**
- 6502-specific signal
- Falling edge sets overflow flag
- Arduino can trigger this for testing

---

### Power Connections

| Signal | Arduino Mega | Z80 Pin | 6502 Pin | 62256 Pin | Notes |
|--------|--------------|---------|----------|-----------|-------|
| **+5V** | **5V** | 11 | 8 | 28 (Vcc) | From Arduino 5V pin |
| **GND** | **GND** | 29 | 1, 21 | 14 (Vss) | Common ground |

**Important:**
- Z80: Vcc=pin 11, Vss=pin 29
- 6502: Vcc=pin 8, Vss=pins 1 AND 21 (both must be grounded!)
- 62256: Vcc=pin 28, Vss=pin 14
- All ICs use +5V ±10%

---

## Port Summary for Firmware

### Arduino Port Configurations

| Port | Pins | Current Use | Direction | Connected To |
|------|------|-------------|-----------|--------------|
| **PORTA** | 22-29 | A0-A7 | OUTPUT | Address bus low |
| **PORTC** | 30-37 | A8-A15 | OUTPUT | Address bus high |
| **PORTL** | 42-49 | D0-D7 | BI | Data bus (DDR controlled) |
| **PORTG** | 38-41 | Control inputs | Mixed | /WR, /RD, /IORQ, /MREQ |
| **PORTH** | 6-9 | Control | Mixed | /M1, /RFSH, /BUSACK, /RESET |
| **PORTB** | 10-13 | Control outputs | OUTPUT | /WAIT, /INT, /NMI, /BUSREQ |
| **PORTE** | 2, 5 | Clock, /HALT | Mixed | CLK (Timer3), /HALT |
| **PORTD** | 18, 20-21 | 6502 specific | Mixed | S.O., Φ2, Φ1 |

---

## Firmware Control Logic

### IC Selection Command (UART)

```
MODE Z80    - Configure pins for Z80 testing
MODE 6502   - Configure pins for 6502 testing
MODE 62256  - Configure pins for 62256 testing
```

### Pin Reconfiguration Required

#### When switching to Z80 mode:
- PG0-PG3: INPUT (monitor /MREQ, /IORQ, /RD, /WR)
- PH3: INPUT (monitor /M1)
- PB4: OUTPUT (control /WAIT)
- PE3: OUTPUT (Timer3 clock to Z80)

#### When switching to 6502 mode:
- PG2: INPUT (monitor R/W) - **Invert logic in firmware**
- PG3: Not used (tri-state or HIGH)
- PH3: INPUT (monitor SYNC) - **Invert logic in firmware**
- PB4: OUTPUT (control RDY) - **Invert logic: HIGH=ready**
- PE3: OUTPUT (Timer3 clock to 6502 Φ0)
- PD0-PD1: INPUT (monitor Φ1, Φ2)
- PD3: OUTPUT (control S.O.)

#### When switching to 62256 mode:
- PG0: OUTPUT (control /CS)
- PG2: OUTPUT (control /OE)
- PG3: OUTPUT (control /WE)
- All other control pins: Not used (can tri-state)
- PE3: Not used (no clock)

---

## Signal Logic Inversions

**CRITICAL:** Some signals have inverted logic between ICs!

| Arduino Pin | Z80 Logic | 6502 Logic | 62256 Logic | Note |
|-------------|-----------|------------|-------------|------|
| PG2 | /RD (L=read) | R/W (H=read) | /OE (L=enable) | **6502 INVERTED** |
| PH3 | /M1 (L=fetch) | SYNC (H=fetch) | — | **6502 INVERTED** |
| PB4 | /WAIT (L=wait) | RDY (H=ready) | — | **6502 INVERTED** |

**Firmware must account for these inversions when switching modes!**

---

## Testing Procedures

### Z80 Testing Mode
1. Insert Z80 in Z80 socket
2. Ensure 6502 and 62256 sockets are EMPTY
3. Send `MODE Z80` command
4. Firmware configures all pins for Z80 operation
5. Use existing Z80 test commands (LOAD, RUN, DUMP, etc.)

### 6502 Testing Mode
1. Insert 6502 in 6502 socket
2. Ensure Z80 and 62256 sockets are EMPTY
3. Send `MODE 6502` command
4. Firmware reconfigures pins for 6502 operation
5. New 6502 test commands available (to be implemented)

### 62256 Testing Mode
1. Insert 62256 in 62256 socket
2. Ensure Z80 and 6502 sockets are EMPTY
3. Send `MODE 62256` command
4. Firmware reconfigures pins for SRAM operation
5. Arduino directly controls address, data, /CS, /OE, /WE
6. Test read/write operations across full 32KB

---

## PCB Design Notes

### Physical Socket Placement
- **Z80 Socket:** 40-pin DIP, centered on right side
- **6502 Socket:** 40-pin DIP, centered on left side
- **62256 Socket:** 28-pin DIP, centered below CPUs
- All sockets: Pin 1 indicator clearly marked

### Shared Trace Routing
- Address bus (A0-A15): Route from Arduino → all three sockets
- Data bus (D0-D7): Route from Arduino → all three sockets
- Clock signal: PE3 → Z80 pin 6 AND 6502 pin 37
- Reset signal: PH6 → Z80 pin 26 AND 6502 pin 40
- Ground plane: Connect all Vss/GND pins

### Trace Priority (shortest/thickest first)
1. Power (+5V, GND) - thick traces
2. Clock signals - short, straight traces
3. Address bus - grouped routing
4. Data bus - grouped routing, equal lengths
5. Control signals - standard width

### Decoupling Capacitors Required
- 0.1µF ceramic capacitor at EACH IC's Vcc-GND (as close as possible)
- 10µF electrolytic capacitor at Arduino 5V input
- Total: 4 capacitors minimum (3 ICs + Arduino)

### Pull-up/Pull-down Resistors
- All unused inputs: 10kΩ pull-up to +5V (especially when socket empty)
- Reset lines: 10kΩ pull-up to +5V (already in schematic)
- Data bus: No pull-ups (actively driven or tri-state)

### Test Points (Recommended)
- TP1: +5V
- TP2: GND
- TP3: CLK (PE3)
- TP4: /RESET (PH6)
- TP5-TP12: D0-D7 (data bus)
- TP13-TP28: A0-A15 (address bus)

---

## New Arduino Pin Assignments Summary

| Pin | Port | Use | Z80 | 6502 | 62256 | Direction |
|-----|------|-----|-----|------|-------|-----------|
| 18 | PD3 | S.O. control | — | S.O. (38) | — | OUT |
| 20 | PD1 | Φ2 monitor | — | Φ2 (39) | — | IN |
| 21 | PD0 | Φ1 monitor | — | Φ1 (3) | — | IN |

**Total new pins used:** 3 (all for 6502-specific signals)

---

## Compatibility Matrix

| Feature | Z80 | 6502 | 62256 | Notes |
|---------|-----|------|-------|-------|
| Address Bus | 16-bit | 16-bit | 15-bit | Fully shared, A15 unused for 62256 |
| Data Bus | 8-bit | 8-bit | 8-bit | Fully shared |
| Clock | Arduino | Arduino | None | Same Timer3 output |
| Reset | Shared | Shared | /CS | Common reset signal |
| Read/Write | Separate | Combined | Separate | Logic inversion required |
| Interrupts | /INT, /NMI | IRQ, NMI | None | Shared pins |
| Special Signals | Many | Few | None | Z80 has most unique signals |

---

## Bill of Materials (BOM) - IC Sockets

| Component | Quantity | Type | Notes |
|-----------|----------|------|-------|
| 40-pin DIP socket | 2 | Standard or ZIF | For Z80 and 6502 |
| 28-pin DIP socket | 1 | Standard or ZIF | For 62256 |
| Arduino Mega 2560 | 1 | — | Main controller |
| 0.1µF capacitor | 3 | Ceramic | One per IC socket |
| 10µF capacitor | 1 | Electrolytic | Arduino power |
| 10kΩ resistor | 20+ | 1/4W | Pull-ups for unused pins |

---

## Firmware Implementation Checklist

### Phase 1: Basic Mode Switching
- [ ] Implement `MODE` command parser
- [ ] Create pin reconfiguration functions for each IC
- [ ] Add logic inversion handling (PG2, PH3, PB4)
- [ ] Test mode switching without ICs

### Phase 2: 62256 Testing
- [ ] Implement direct memory read/write
- [ ] Add address range validation (0x0000-0x7FFF)
- [ ] Create walking 1s/0s test patterns
- [ ] Verify /CS, /OE, /WE timing

### Phase 3: 6502 Testing
- [ ] Implement 6502 bus cycle handler
- [ ] Add R/W signal interpretation (inverted logic)
- [ ] Monitor SYNC for opcode fetch detection
- [ ] Handle RDY signal for wait states
- [ ] Monitor Φ1/Φ2 clock outputs

### Phase 4: Integration
- [ ] Test all three modes
- [ ] Add mode status to STATUS command
- [ ] Update HELP command with new commands
- [ ] Create test programs for each IC

---

**Last Updated:** 2026-01-03
**Status:** Complete pin mapping for multi-IC tester
**Revision:** 1.0
