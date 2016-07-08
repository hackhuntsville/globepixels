from snappy_mpu6050 import *

@setHook(HOOK_STARTUP)
def setup():
    init_imu(27, 29)

@setHook(HOOK_100MS)
def timer():
    x = get_imu_accel("x")
    y = get_imu_accel("y")
    z = get_imu_accel("z")

    if x > 15000:
        color = "purple"
    elif x < -15000:
       color = "red"
    elif z > 15000:
       color = "white"
    elif z < -15000:
       color = "green"
    elif y > 15000:
       color = "yellow"
    elif y < -15000:
       color = "orange"
    else:
       return
    mcastRpc(1,3,"report", color)
