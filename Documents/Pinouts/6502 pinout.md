6502 40-Pin DIP Pinout Table
Pin	Symbol	Type	Description
1	Vss	Power	Ground (0V). Note: VPB on modern WDC chips
2	RDY	Input	Ready; pauses the CPU when low
3	Φ1 (out)	Output	Phase 1 clock output (inverted Φ2)
4	IRQ	Input	Interrupt Request (active low, maskable)
5	NC	—	No Connection. Note: MLB (Memory Lock) on WDC chips
6	NMI	Input	Non-Maskable Interrupt (active low)
7	SYNC	Output	Synchronize; high during opcode fetch
8	Vcc	Power	Supply Voltage (+5V)
9–20	A0–A11	Output	Address Bus (Lower 12 bits)
21	Vss	Power	Ground (0V)
22–25	A12–A15	Output	Address Bus (Upper 4 bits)
26–33	D7–D0	Bidirectional	8-bit Data Bus (D7 is Pin 26, D0 is Pin 33)
34	R/W	Output	Read/Write (High=Read, Low=Write)
35	NC	—	No Connection
36	NC	—	No Connection. Note: BE (Bus Enable) on WDC chips
37	Φ0 (in)	Input	Phase 0 clock input
38	S.O.	Input	Set Overflow flag (active low)
39	Φ2 (out)	Output	Phase 2 clock output
40	RES	Input	Reset (active low, must be held for 2+ cycles)