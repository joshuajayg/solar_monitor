The board uses six LTC4151-2 ICs, an ESP8266 running NODEMCU and one DROK buck converter.

The wires run from the board to the current shunts needs to be twisted pair wires.  This cuts down on EMI by common mode rejection.  For short runs, CAT5 wire works fine.

When adding a external input to D3, there is a voltage divider used to drop the voltage to 3.3V at the MCU.  You need to calculate these values as follows:
Vout = (Vin * R3) / (R4 + R3)  
