#!/usr/bin/env python

import serial
import numpy as np
import matplotlib.pyplot as plt
import time
import sys

def getData():
    while True:
        raw_lines = Arduino.readline()
        try:
            #readlines and convert to numpy array. skip empty lines and bogus lines
            if raw_lines == '':
                time.sleep(.005)
                continue
            nums = raw_lines.strip().split()
            numArray = np.array([float(i)/1000. for i in nums],ndmin = 2)
            if numArray.shape[1] != 2: #expect three numbers
                time.sleep(.005)
                continue
            return numArray
        except ValueError:
            print 'bogus data',raw_lines
            time.sleep(.005)

global Arduino
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

#start capturing lines
plot_data = getData()

#create figure
window = 20
fig = plt.figure()
axes = fig.add_subplot(111)
line, = plt.plot(plot_data[:,0],plot_data[:,1])
axes.set_xlim(plot_data[-1,0] - window/2,plot_data[-1,0] + window/2)
axes.set_ylim(0, 60)
plt.ion()
plt.show()

while True:
    try:
        data = getData()
        plot_data = np.append(plot_data,data, axis = 0)
        axes.set_xlim(plot_data[-1,0] - window/2,plot_data[-1,0] + window/2)
        line.set_xdata(plot_data[:,0])
        line.set_ydata(plot_data[:,1])
        print(data)
        plt.draw()
    except KeyboardInterrupt:
        Arduino.close()
    
        plt.ioff()
        plt.clf()
        plt.plot(plot_data[:,0],plot_data[:,1])
        try:
            plt.show()
        except KeyboardInterrupt:
            exit(0)
        exit(0)
