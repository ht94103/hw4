import paho.mqtt.client as paho
import time
import matplotlib.pyplot as plt
import numpy as np
import serial

# https://os.mbed.com/teams/mqtt/wiki/Using-MQTT#python-client

# MQTT broker hosted on local machine
mqttc = paho.Client()


serdev = '/dev/ttyACM0'
s = serial.Serial(serdev, 9600)
data=[]
x=[]
y=[]
z=[]
ind = []
t=[]
timestamp=[]
i = 0

# Settings for connection
# TODO: revise host to your ip
host = "192.168.0.111" 
topic= "Mbed"

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))


def on_message(mosq, obj, msg):
	mess=str(msg.payload)
# save data after reading the first indicator
while (1):
	line = s.readline()
	if float(line) == 999.5:
		line = s.readline()
		ind.append(float(line))
		break;
	if float(line) == 111.5:
		line = s.readline()
		ind.append(float(line))
		break;

t0 = time.time()
while (1):
	sample_time = time.time()
	if sample_time-t0 < 20:
		t.append(sample_time-t0)
		line = s.readline()
		data.append(float(line))
		line = s.readline()
		data.append(float(line))
		line = s.readline()
		data.append(float(line))
		line = s.readline()
		ind.append(float(line))

	else :
		break

ind.append(ind[len(ind) - 1])

for i in range (int(len(ind) - 1)):
	p = ind[i]
	k = ind[i + 1]
	if k != p:   #save data to x, y, z lists when the indicator changes
		x.append(data[3*i])
		y.append(data[3*i + 1])
		z.append(data[3*i + 2])
		timestamp.append(t[i])

def on_subscribe(mosq, obj, mid, granted_qos):

    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)

plt.plot(timestamp, x, color = "blue", label = "x-acc")
plt.ylim(-1.5, 1.5)
plt.plot(timestamp, y, color = "red", label = "y-acc")
plt.plot(timestamp, z, color = "green", label = "z-acc")
plt.xlabel("timestamp")
plt.ylabel("Acc Value")
plt.legend()

plt.show()
s.close
