#!/usr/bin/env ipython

import serial
import numpy as np
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
import time
from sys import exit, stderr

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

def plotData(pltData):
    curve.setData(pltData)
    plt.setXRange(pltData[0,0] - 4*window/5,pltData[0,0] + window/5)

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
    stderr.write('Arduino not found \n')
    exit(0)

#allocate data array
sampleTime = 120
frq = 100
numValues = 5
data = np.zeros((sampleTime*frq,numValues))

#Set up GUI window
app = QtGui.QApplication([])
win = pg.GraphicsWindow(title="Encoder Data")
win.resize(1000,800)
win.setWindowTitle('Encoder Data')

#create figure
window = 20
plt = win.addPlot()
curve = plt.plot(pen = 'b')
plt.setYRange(-5, 60)
plt.enableAutoRange('xy', False)

def updatePlot():
    curve.setData(data[0:i+1,0:2])
    plt.setXRange(data[0,0] - 4*window/5,data[0,0] + window/5)
    
timer = QtCore.QTimer()
timer.timeout.connect(updatePlot)
timer.start(50)

#gather streaming data
i = 0
while True:
    try:
        #add new data to array. If array is too small, make it bigger and add data
        newData = getData()
        print newData
        try:
            data[i,:] = newData
        except IndexError:
            data = np.append(data,np.zeros((sampleTime*frq,numValues)),axis = 0)
            data[i,:] = newData
        i = i + 1

    except KeyboardInterrupt:
        Arduino.close()
        try:
            plt.enableAutoRange('xy', True)
            timer.stop()
        except KeyboardInterrupt:
            exit(0)
        finally:
            exit(0)

'''
data = np.random.normal(size=(10,1000))
ptr = 0
def update():
    global curve, data, ptr, p6
    curve.setData(data[ptr%10])
    if ptr == 0:
        p6.enableAutoRange('xy', False)  ## stop auto-scaling after the first data set is plotted
    ptr += 1
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(50)
'''
