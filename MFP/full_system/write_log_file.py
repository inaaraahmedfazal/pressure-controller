import serial
import csv

def readserial(port, baud, fname):
    s = serial.Serial(port, baud, timeout=1)
    q1 = []
    q2 = []
    q3 = []
    q4 = []
    notEnd = True
    f = open("log1.txt", 'w')
    while notEnd:
        data = str(s.readline().decode().strip())
        f.write(f'{data}\n')
        print(data)
        if data and "end" in data:
            notEnd = False

    f.close()

if __name__ == '__main__':
    readserial('COM8', 9600, 'fullSysTest.txt')
