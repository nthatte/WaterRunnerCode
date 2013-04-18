import serial
import numpy as np
import matplotlib.pyplot as plt
import time

def getData():
    while True:
        raw_lines = Arduino.readline()
        try:
            #readlines and convert to numpy array. skip empty lines and bogus lines
            if raw_lines == '':
                time.sleep(.005)
                continue
            nums = raw_lines.strip().split()
            numArray = np.array([float(i) for i in nums], ndmin = 2)
            return numArray
        except ValueError:
            print 'bogus data',raw_lines
            time.sleep(.005)

global Arduino
try:
    Arduino = serial.Serial(
        port='/dev/ttyACM0',
        baudrate=9600,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.1,
        xonxoff=0,
        rtscts=0,
        interCharTimeout=None
    )
    #burn initial buffered output
    burn = Arduino.readline()
    while burn != '':
        burn = Arduino.readline()

except serial.serialutil.SerialException:
    print('Arduino not found')
    ser = None

#start capturing lines
plot_data = getData()
print(plot_data)

numSamples = 400
#create figure
fig = plt.figure()
axes = fig.add_subplot(111)
line, = plt.plot(plot_data[:,0],plot_data[:,1])
axes.set_xlim(0,numSamples/100 + 2)
axes.set_ylim(-1,1)
plt.ion()
plt.show()

s = 0
while s < numSamples:
	data = getData()
	plot_data = np.append(plot_data,data, axis = 0)
	'''
	plt.clf()
	line, = plt.plot(plot_data[:,0],plot_data[:,1])
	plt.show()
	'''
	line.set_xdata(plot_data[:,0])
	line.set_ydata(plot_data[:,1])
	plt.draw()
	s = s + 1
Arduino.close()

plt.ioff()
plt.clf()
plt.plot(plot_data[:,0],plot_data[:,1])
axes.set_xlim(0,3)
axes.set_ylim(-1,1)
plt.show()
