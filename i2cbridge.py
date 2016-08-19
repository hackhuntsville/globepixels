from pyduinoincludes import *

SCL = D5  #Uno A5 --> Pyduino D5
SDA = D4  #Uno A4 --> Pyduino D4

I2C_ADDR = "\x10"

@setHook(HOOK_STARTUP)
def init():
    i2cInit(True, SCL, SDA)

def report(color):

    command = "!"
    if color == "purple":
        command = "1"
    elif color == "red":
        command = "2"
    elif color == "green":
        command = "3"
    elif color == "blue":
        command = "4"
    elif color == "yellow":
        command = "5"
    elif color == "orange":
        command = "6"

    i2cWrite(I2C_ADDR + command, 1000, True)
    return command

