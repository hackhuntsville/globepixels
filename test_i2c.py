#from pyduinoincludes import *

SCL = 27 
SDA = 29

def send(addr, data):
    i2cInit(True, SCL, SDA)
    addr = "\x10"
    i2cWrite(addr+data, 10, True)
    return addr+data

