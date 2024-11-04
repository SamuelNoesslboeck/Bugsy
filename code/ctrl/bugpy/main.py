import serial
from threading import Thread
from time import sleep

s = serial.Serial("COM8", 115200)

def read():
    while True:
        print(s.readall().decode("ascii"))
        

thr = Thread(target=read)
thr.start()

while True:
    sleep(1)
    s.write(b"\x01")