from snappy_mpu6050 import *

MAX_STEADY = 6
MAX_DEDUPE = 3
dedupe_count = MAX_STEADY
current_color = "purple"


@setHook(HOOK_STARTUP)
def setup():
    setPinDir(17, True)
    writePin(17, False)
    init_imu(27, 29)

@setHook(HOOK_10MS)
def timer():
    global MAX_DEDUPE, dedupe_count, current_color

    x = get_imu_accel("x")
    y = get_imu_accel("y")
    z = get_imu_accel("z")

    if x > 15000:
        new_color = "purple"
    elif x < -15000:
        new_color = "red"
    elif z > 15000:
        new_color = "blue"
    elif z < -15000:
        new_color = "green"
    elif y > 15000:
        new_color = "yellow"
    elif y < -15000:
        new_color = "orange"
    else:
        return

    if current_color != new_color:
        dedupe_count = MAX_STEADY
        current_color = new_color
    elif dedupe_count > MAX_DEDUPE:
        dedupe_count -= 1
    elif dedupe_count > 0:
        dedupe_count -= 1
        mcastRpc(1,3,"report", current_color)

