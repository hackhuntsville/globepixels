from snappy_mpu6050 import *

MAX_STEADY = 6
MAX_DEDUPE = 3
STARTUP_DELAY = 10

dedupe_count = MAX_STEADY
current_color = "purple"

startup_count = 0
init_done = False

@setHook(HOOK_STARTUP)
def setup():
    setPinDir(17, True)
    writePin(17, False)
    init_imu(27, 29)

def get_color():
    return current_color

@setHook(HOOK_1S)
def update():
    global STARTUP_DELAY, startup_count, init_done
    if init_done:
      mcastRpc(1,3,"report", current_color)
    elif startup_count < STARTUP_DELAY:
        startup_count += 1
    elif startup_count == STARTUP_DELAY:
        setup()
        init_done = True

@setHook(HOOK_10MS)
def timer():
    global MAX_DEDUPE, dedupe_count, current_color, init_done

    if init_done:

        x = get_imu_accel("x")
        y = get_imu_accel("y")
        z = get_imu_accel("z")

        if x > 15000:
            new_color = "orange"
        elif x < -15000:
            new_color = "yellow"
        elif z > 15000:
            new_color = "red"
        elif z < -15000:
            new_color = "purple"
        elif y > 15000:
            new_color = "blue"
        elif y < -15000:
            new_color = "green"
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

