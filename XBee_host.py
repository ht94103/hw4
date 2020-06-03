import serial

import time


# XBee setting

serdev = '/dev/ttyUSB0'

s = serial.Serial(serdev, 9600)


s.write("+++".encode())

char = s.read(2)

print("Enter AT mode.")

print(char.decode())


s.write("ATMY 0x187\r\n".encode())

char = s.read(3)

print("Set MY 0x187.")

print(char.decode())


s.write("ATDL 0x178\r\n".encode())

char = s.read(3)

print("Set DL 0x178.")

print(char.decode())


s.write("ATID 0x3\r\n".encode())

char = s.read(3)

print("Set PAN ID 0x3.")

print(char.decode())


s.write("ATWR\r\n".encode())

char = s.read(3)

print("Write config.")

print(char.decode())


s.write("ATMY\r\n".encode())

char = s.read(4)

print("MY :")

print(char.decode())


s.write("ATDL\r\n".encode())

char = s.read(4)

print("DL : ")

print(char.decode())


s.write("ATCN\r\n".encode())

char = s.read(3)

print("Exit AT mode.")

print(char.decode())


print("start sending RPC")

s.write("/myled1/write 1\r".encode())

s.close()