import serial

import time

import matplotlib.pyplot as plt

import numpy as np


# XBee setting

serdev = '/dev/ttyUSB0'

s = serial.Serial(serdev, 9600)

data = np.arange(0, 20, 1)


s.write("+++".encode())

char = s.read(2)

print("Enter AT mode.")

print(char.decode())


s.write("ATMY <BASE_MY>\r\n".encode())

char = s.read(3)

print("Set MY <BASE_MY>.")

print(char.decode())


s.write("ATDL <BASE_DL>\r\n".encode())

char = s.read(3)

print("Set DL <BASE_DL>.")

print(char.decode())


s.write("ATID <PAN_ID>\r\n".encode())

char = s.read(3)

print("Set PAN ID <PAN_ID>.")

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

for i in range 10:

    s.write("/myled1/write 1\r".encode())

    line = s.readline()

    data[i] = int(line)

    time.sleep(1)

    s.write("/myled1/write 0\r".encode())

    line = s.readline()

    data[i] = int(line)

    time.sleep(1)


Time = np.arange(0, 20, 1)


plt.plot(Time, x, color = "green")

#plt.ylim(-1.5, 1.5)

plt.xlabel("Time")

plt.ylabel("Number")


plt.show()

s.close