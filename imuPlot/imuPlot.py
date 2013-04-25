#!/usr/bin/env python

import serial
import struct
from bitstring import BitArray
import bitstring
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import time
import sys
from quaternion import*

def getData():
    while True:
        #read until packet start byte is encountered
        try:
            byte = BitArray(bytes = Arduino.read(), length = 8)
            if byte.hex == 'f7':
                #read time, quaternion, and checksum
                timeArray = BitArray(bytes = Arduino.read(size = 4), length = 8*4)
                quatArray = BitArray(bytes = Arduino.read(size = 16), length = 8*16)
                checksum1 = BitArray(bytes = Arduino.read(), length = 8).uint

                #if checksum validates convert time and quaternion ByteArrays to 
                #unsigned long and floating points respectively
                checksum2 = calcChecksum(timeArray + quatArray)
                if checksum1 == checksum2:
                    curtime = struct.unpack_from('<L',bytearray(timeArray.bytes))
                    quat = struct.unpack_from('>ffff',bytearray(quatArray.bytes))
                    return (curtime, quat)
        except KeyboardInterrupt:
            exit(0)
        except bitstring.CreationError:
            continue

global Arduino
try:
    #setup arduino
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

#axes for plot
x = (-1, 0, 0)
y = (0, -1, 0)
z = (0, 0, 1)

#initialize plot
fig = plt.figure()
axes = fig.gca(projection = '3d')
(curtime, quat) = getData()
x1 = qv_mult(quat, x)
y1 = qv_mult(quat, y)
z1 = qv_mult(quat, z)

linex, = axes.plot([0, x1[0]],[0, x1[1]],[0, x1[2]])
liney, = axes.plot([0, y1[0]],[0, y1[1]],[0, y1[2]])
linez, = axes.plot([0, z1[0]],[0, z1[1]],[0, z1[2]])

axes.set_xlabel('x');        
axes.set_ylabel('y');        
axes.set_zlabel('z');        
plt.ion()
plt.show()

while True:
    try:
        #update plot
        (curtime, quat) = getData()

        x1 = qv_mult(quat, x)
        y1 = qv_mult(quat, y)
        z1 = qv_mult(quat, z)
        
        linex.set_xdata([0, x1[0]])
        linex.set_ydata([0, x1[1]])
        linex.set_3d_properties(zs = [0, x1[2]])

        liney.set_xdata([0, y1[0]])
        liney.set_ydata([0, y1[1]])
        liney.set_3d_properties(zs = [0, y1[2]])

        linez.set_xdata([0, z1[0]])
        linez.set_ydata([0, z1[1]])
        linez.set_3d_properties(zs = [0, z1[2]])

        plt.draw()
    except KeyboardInterrupt:
        exit(0)
