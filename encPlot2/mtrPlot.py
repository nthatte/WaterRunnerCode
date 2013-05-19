#!/usr/bin/env python

import serial
import numpy as np
import matplotlib.pyplot as plt
import time
import sys

#polls ardunio until legitimate data is received, then parses and returns data
def getData():
    while True:
        raw_lines = Arduino.readline()
        try:
            #readlines and convert to numpy array. skip empty lines and bogus lines
            stringNums = raw_lines.strip().split()
            intArray = np.array([int(i) for i in stringNums])

            #check checksum (last value) in array
            #if np.sum(intArray[0:-1])%256 != intArray[-1]:
            #    continue
            numArray = intArray.astype(np.float64)
            numArray[0] = numArray[0]/1e6
            numArray[1] = numArray[1]*2*np.pi*1e6/0x4000/1024
        except (ValueError, IndexError):
            pass
        else:
            return numArray

# setup arduino serial port
try:
    Arduino = serial.Serial(
        port='/dev/tty.usbmodem1421',
        baudrate=115200,
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

#allocate data array
sampleTime = 120
frq = 100
numValues = 5
data = np.zeros((sampleTime*frq,numValues))

#start capturing data
data[0,:] = getData()

#create figure
window = 20
fig = plt.figure()
axes = fig.add_subplot(111)
line, = plt.plot(data[0,:],data[1,:])
axes.set_xlim(data[0,0] - 4*window/5,data[0,0] + window/5)
axes.set_ylim(-5, 100)
plt.ion()
plt.show()

#plot streaming data
i = 1
while True:
    try:
        newData = getData()
        print newData
        #add new data to array. If array is too small, make it bigger and add data
        try:
            data[i,:] = getData()
        except IndexError:
            data = np.append(data,np.zeros((sampleTime*frq,numValues)),axis = 0)
            data[i,:] = getData()

        axes.set_xlim(data[i,0] - 4*window/5,data[i,0] + window/5)
        line.set_xdata(data[0:i,0])
        line.set_ydata(data[0:i,1])
        plt.draw()
        i = i + 1
    except KeyboardInterrupt:
        Arduino.close()
        plt.ioff()
        plt.clf()
        line, = plt.plot(data[0:i,0],data[0:i,1])
        try:
            plt.show()
        except KeyboardInterrupt:
            exit(0)
        exit(0)


