# Z80 CPU Pinout Reference

## DIP-40 Package Pinout

| Pin | Signal   | Type   | Function                              |
|-----|----------|--------|---------------------------------------|
| 1   | A11      | Output | Address Bus (A0–A15)                  |
| 2   | A12      | Output | Address Bus                           |
| 3   | A13      | Output | Address Bus                           |
| 4   | A14      | Output | Address Bus                           |
| 5   | A15      | Output | Address Bus                           |
| 6   | CLK      | Input  | System Clock                          |
| 7   | D4       | I/O    | Data Bus (D0–D7)                      |
| 8   | D3       | I/O    | Data Bus                              |
| 9   | D5       | I/O    | Data Bus                              |
| 10  | D6       | I/O    | Data Bus                              |
| 11  | +5V      | Power  | Vcc power supply                      |
| 12  | D2       | I/O    | Data Bus                              |
| 13  | D7       | I/O    | Data Bus                              |
| 14  | D0       | I/O    | Data Bus                              |
| 15  | D1       | I/O    | Data Bus                              |
| 16  | /INT     | Input  | Interrupt Request (Active Low)        |
| 17  | /NMI     | Input  | Non-Maskable Interrupt (Active Low)   |
| 18  | /HALT    | Output | CPU in halt state (Active Low)        |
| 19  | /MREQ    | Output | Memory Request (Active Low)           |
| 20  | /IORQ    | Output | I/O Request (Active Low)              |
| 21  | /RD      | Output | Read signal (Active Low)              |
| 22  | /WR      | Output | Write signal (Active Low)             |
| 23  | /BUSACK  | Output | Bus Acknowledge (Active Low)          |
| 24  | /WAIT    | Input  | CPU wait states (Active Low)          |
| 25  | /BUSRQ   | Input  | Bus Request (Active Low)              |
| 26  | /RESET   | Input  | System Reset (Active Low)             |
| 27  | /M1      | Output | Machine Cycle One (Active Low)        |
| 28  | /RFSH    | Output | Dynamic RAM Refresh (Active Low)      |
| 29  | GND      | Power  | Ground connection                     |
| 30  | A0       | Output | Address Bus                           |
| 31  | A1       | Output | Address Bus                           |
| 32  | A2       | Output | Address Bus                           |
| 33  | A3       | Output | Address Bus                           |
| 34  | A4       | Output | Address Bus                           |
| 35  | A5       | Output | Address Bus                           |
| 36  | A6       | Output | Address Bus                           |
| 37  | A7       | Output | Address Bus                           |
| 38  | A8       | Output | Address Bus                           |
| 39  | A9       | Output | Address Bus                           |
| 40  | A10      | Output | Address Bus                           |

## Signal Groups

### Address Bus (16-bit)
- **Pins 30-40, 1-5**: A0-A15
- **Type**: Output from Z80
- **Function**: 16-bit address for memory/IO access

### Data Bus (8-bit)
- **Pins 14, 15, 12, 8, 7, 9, 10, 13**: D0-D7
- **Type**: Bidirectional
- **Function**: 8-bit data transfer

### Control Signals (Outputs from Z80)
- **Pin 19**: /MREQ - Memory Request
- **Pin 20**: /IORQ - I/O Request
- **Pin 21**: /RD - Read
- **Pin 22**: /WR - Write
- **Pin 27**: /M1 - Machine Cycle 1 (Opcode Fetch)
- **Pin 28**: /RFSH - DRAM Refresh
- **Pin 23**: /BUSACK - Bus Acknowledge
- **Pin 18**: /HALT - Halt State

### Control Signals (Inputs to Z80)
- **Pin 6**: CLK - System Clock
- **Pin 26**: /RESET - Reset
- **Pin 24**: /WAIT - Wait State
- **Pin 25**: /BUSRQ - Bus Request
- **Pin 16**: /INT - Maskable Interrupt
- **Pin 17**: /NMI - Non-Maskable Interrupt

### Power
- **Pin 11**: +5V (Vcc)
- **Pin 29**: GND

## Notes

1. All signals with "/" prefix are **active LOW**
2. The Z80 requires a single +5V supply (±5% tolerance)
3. Data bus pins are bidirectional and require external buffering for high-speed operation
4. Address bus is unidirectional output from Z80
5. Control signals use TTL logic levels (0V = LOW, +5V = HIGH)

---

**Reference**: Z80 CPU Technical Manual
**Package**: DIP-40 (Dual In-line Package)
**Last Updated**: 2026-01-02
