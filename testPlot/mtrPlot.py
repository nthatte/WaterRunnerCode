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
        port='/dev/tty.usbmodem1421',
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

#create figure
fig = plt.figure()
axes = fig.add_subplot(111)
axes.set_title('Motor Velocity [r/s]', size=12)
line, = plt.plot([],[])
plt.show(block = False)

time.sleep(0.1)
#capture 10s of data
numSamples = 100
s = 0
while s < numSamples:
    data = getData()
    plot_data = np.append(plot_data,data, axis = 0)
    plt.clf()
    line, = plt.plot(plot_data[:,0],plot_data[:,1])
    plt.show(block = False)
    '''
    line.set_xdata(plot_data[:,0])
    line.set_ydata(plot_data[:,1])
    plt.draw()
    plt.pause(0.01)
    '''
    print(data)
    s = s + 1
Arduino.close()
plt.close()
plt.ioff()
plt.plot(plot_data[:,0],plot_data[:,1])
plt.show()
