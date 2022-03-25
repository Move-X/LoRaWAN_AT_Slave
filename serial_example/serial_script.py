## uplink test on EU868 band, set mandatory keys before running this ##

# tested with python3 (version 3.10.2)

import serial # install with "pip install pyserial", tested with pyserial packet (3.5 version)


## SERIAL SETTINGS
PORT = 'XXXX' # SET HERE SERIAL PORT
BAUDRATE = 9600
TIMEOUT = 2

# init and open serial port
serial_port = serial.Serial(port=PORT, baudrate=BAUDRATE, timeout=TIMEOUT)


## RESET BOARD ##
tx_string = "ATZ\n"
serial_port.write(tx_string.encode())
print("> " + tx_string)

# receive string
rx_string = serial_port.readline().decode('utf-8').rstrip()
print(rx_string)

# receive continuously until expected string received
while rx_string != "":
    rx_string = serial_port.readline().decode('utf-8').rstrip()
    print(rx_string)
    if rx_string == 'Default':
        break


## KEYS SETTING ##

# band setting
tx_string = "AT+BAND=5\n" # EU868 BAND
serial_port.write(tx_string.encode())
print("> " + tx_string)

rx_string = serial_port.readline().decode('utf-8').rstrip()
print(rx_string)

while rx_string != "":
    rx_string = serial_port.readline().decode('utf-8').rstrip()
    print(rx_string)
    if rx_string == 'Default':
        break

# deui setting
tx_string = "AT+DEUI=XX:XX:XX:XX:XX:XX:XX:XX\n" # SET HERE DEUI KEY
serial_port.write(tx_string.encode())
print("> " + tx_string)

rx_string = serial_port.readline().decode('utf-8').rstrip()
print(rx_string)

# appeui setting
tx_string = "AT+APPEUI=XX:XX:XX:XX:XX:XX:XX:XX\n" # SET HERE APPEUI KEY
serial_port.write(tx_string.encode())
print("> " + tx_string)

rx_string = serial_port.readline().decode('utf-8').rstrip()
print(rx_string)

# appkey setting
tx_string = "AT+APPKEY=XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX\n" # SET HERE APPKEY KEY
serial_port.write(tx_string.encode())
print("> " + tx_string)

rx_string = serial_port.readline().decode('utf-8').rstrip()
print(rx_string)

# nwkkey setting
tx_string = "AT+NWKKEY=XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX\n" # SET HERE NWKKEY
serial_port.write(tx_string.encode())
print("> " + tx_string)

rx_string = serial_port.readline().decode('utf-8').rstrip()
print(rx_string)


## JOIN ## 

tx_string = "AT+JOIN=1\n" # OTAA JOIN TYPE
serial_port.write(tx_string.encode())
print("> " + tx_string)

while True:
    rx_string = serial_port.readline().decode('utf-8').rstrip()
    if rx_string != "":
        print(rx_string)
    if rx_string == "+EVT:JOINED":
        break


## SEND PARAMETERS ##

tx_string = "AT+CFGSEND=10:5:0:1000:5:0\n" 
serial_port.write(tx_string.encode())
print("> " + tx_string)

rx_string = serial_port.readline().decode('utf-8').rstrip()
print(rx_string)


## SEND ##

tx_string = "AT+SEND=2:FFFF\n" 
serial_port.write(tx_string.encode())
print("> " + tx_string)

while True:
    rx_string = serial_port.readline().decode('utf-8').rstrip()
    if rx_string != "":
        print(rx_string)
    if rx_string == "Finish or stop send":
        break