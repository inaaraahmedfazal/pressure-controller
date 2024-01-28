import serial
import csv

def readserial(port, baud, fname):
    s = serial.Serial(port, baud, timeout=2)
    q1 = []
    q2 = []
    q3 = []
    q4 = []
    notEnd = True
    while notEnd:
        data = str(s.readline().decode().strip())
        if data and "Starting sample:" in data:
            data1 = str(s.readline().decode().strip())
            q1.append(data1[data1.index(':') + 1:].strip())

            data2 = str(s.readline().decode().strip())
            q2.append(data2[data2.index(':') + 1:].strip())

            data3 = str(s.readline().decode().strip())
            q3.append(data3[data3.index(':') + 1:].strip())

            data4 = str(s.readline().decode().strip())
            q4.append(data4[data4.index(':') + 1:].strip())
        elif data and "end" in data:
            notEnd = False

    with open(fname, 'w', newline='') as csvfile:
        w = csv.writer(csvfile)
        for i in range(len(q1)):
            w.writerow([q1[i], q2[i], q3[i], q4[i]])

if __name__ == '__main__':
    readserial('COM3', 9600, 'trial1.csv')
