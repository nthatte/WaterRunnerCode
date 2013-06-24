#!/usr/bin/env python

import serial
from sys import exit, stderr
import numpy as np
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
from time import sleep

#Set up GUI window
app = QtGui.QApplication([])
win = pg.GraphicsWindow(title="Encoder Data")
win.resize(500,400)
win.setWindowTitle('Encoder Data')

#create figure
window = 2.
pltfreq = 30.
freq = 1000
plt = win.addPlot(title = 'hi')
curve = plt.plot(pen = 'b')
plt.setYRange(-5, 60)
plt.enableAutoRange('xy', False)
i = 0
data = np.zeros((5000,2))

def updatePlot():
    global i, data, window, curve, plt
    low = i - window*freq
    low = 0 if low < 0 else low
    curve.setData(data[low:i,:])
    plt.setXRange(data[i-1,0] - 4*window/5,data[i-1,0] + window/5)
    print data[i-1,:]

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

def getData():
    global i, data, Arduino

    raw_lines = Arduino.readline()
    try:
        #readlines and convert to numpy array. skip empty lines and bogus lines
        stringNums = raw_lines.strip().split()
        intArray = np.array([int(num) for num in stringNums])
        #return if no data
        if intArray.size == 0:
            return

        #check checksum (last value) in array
        #if np.sum(intArray[0:-1])%256 != intArray[-1]:
    # return if data is corrupt (value error)
    except ValueError:
        return

    try:
        data[i,0] = intArray[0]/1.e6
        data[i,1] = intArray[1]*2*np.pi*1e6/0x4000/1024
    except IndexError:
        data = np.append(data,np.zeros(data.shape),axis = 0)
        data[i,0] = intArray[0]/1.e6
        data[i,1] = intArray[1]*2*np.pi*1e6/0x4000/1024
    finally:
        i = i + 1

timer1 = QtCore.QTimer()
timer1.timeout.connect(updatePlot)
timer1.start((1./pltfreq) * 1000)

timer2 = QtCore.QTimer()
timer2.timeout.connect(getData)
timer2.start(1./freq * 1000)

## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
    import sys
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        QtGui.QApplication.instance().exec_()
