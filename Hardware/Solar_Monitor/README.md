The board uses six LTC4151-2 ICs, an ESP8266 running NODEMCU and one DROK buck converter.

#Known issues
    I have some routing issues among the ICs causing noise to propogate between the sensors.  When the inverter load is increased, the auxiliary sensors start indicating over 1 amps of load.  This is a significant issue since it really upsets the state of charge calculations overnight.  
