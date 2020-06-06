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
t0 = time.time()

def on_message(mosq, obj, msg):
	mess=str(msg.payload)
while (1):
	line = s.readline()
	if float(line) == 999.9:
		line = s.readline()
		ind.append(float(line))
		break;
	if float(line) == 111.1:
		line = s.readline()
		ind.append(float(line))
		break;

while (1):
	sample_time = time.time()
	if sample_time-t0 < 20.1:
		line = s.readline()
		data.append(float(line))
		line = s.readline()
		data.append(float(line))
		line = s.readline()
		data.append(float(line))
		line = s.readline()
		ind.append(float(line))
		t.append(sample_time-t0)
	else :
		break

ind.append(ind[len(ind) - 1])
x.append(data[0])
y.append(data[1])
z.append(data[2])
timestamp.append(t[0])

for i in range (int(len(ind) - 1)):
	p = ind[i]
	k = ind[i + 1]
	if k != p:
		x.append(data[3*i])
		y.append(data[3*i + 1])
		z.append(data[3*i + 2])
		timestamp.append(i)

//print(len(x), len(y), len(z), len(timestamp), len(ind))
#print(timestamp)
#print('\n')
#print(data)
    #print(" Message: " + str(msg.payload) + "\n");
	#for i in range (2):
		#s =   str(msg.payload)
		#s1=s[2:-5]
		#s2=s1.split(',')
		#x.append(eval(s2[2]))
	#print(x)

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

# Loop forever, receiving messages
#mqttc.loop_forever()

#print("rc: " + str(rc))



plt.plot(timestamp, x, color = "blue", label = "x-acc")
plt.ylim(-1.5, 1.5)
#plt.xlim(0, 20)
plt.plot(timestamp, y, color = "red", label = "y-acc")
plt.plot(timestamp, z, color = "green", label = "z-acc")
plt.xlabel("timestamp")
plt.ylabel("Acc Value")
plt.legend()

plt.show()
s.close
