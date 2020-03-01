
import usb.core
import time
import numpy as np
import sys
import select
import tty
import termios

def isData():
        return select.select([sys.stdin], [], [], 0) == ([sys.stdin], [], [])

class encodertest:

    def __init__(self):
        self.SPRING_STATE = 0
        self.WALL_STATE = 1
        self.DAMP_STATE = 2
        self.TEXTURE_STATE = 3
        self.READ_SW1 = 4
        self.ENC_READ_REG = 6
        self.TUNE_UP = 7
        self.TUNE_DOWN = 8
        self.dev = usb.core.find(idVendor = 0x6666, idProduct = 0x0003)
        if self.dev is None:
            raise ValueError('no USB device found matching idVendor = 0x6666 and idProduct = 0x0003')
        self.dev.set_configuration()

# AS5048A Register Map
        self.ENC_NOP = 0x0000
        self.ENC_CLEAR_ERROR_FLAG = 0x0001
        self.ENC_PROGRAMMING_CTRL = 0x0003
        self.ENC_OTP_ZERO_POS_HI = 0x0016
        self.ENC_OTP_ZERO_POS_LO = 0x0017
        self.ENC_DIAG_AND_AUTO_GAIN_CTRL = 0x3FFD
        self.ENC_MAGNITUDE = 0x3FFE
        self.ENC_ANGLE_AFTER_ZERO_POS_ADDER = 0x3FFF

    def close(self):
        self.dev = None

    def read_sw1(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.READ_SW1, 0, 0, 1)
        except usb.core.USBError:
            print("Could not send READ_SW1 vendor request.")
        else:
            return int(ret[0])

    def get_angle(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.ENC_READ_REG, 0x3FFF, 0, 2)
        except usb.core.USBError:
            print ("Could not send ENC_READ_REG vendor request.")
        else:
            return (int(ret[0]) + 256 * int(ret[1])) & 0x3FFF

    def toggle_state(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TOGGLE_STATE)
        except usb.core.USBError:
            print("Could not send TOGGLE_STATE vendor request.")

    def spring_state(self):
        try:
            self.dev.ctrl_transfer(0x40, self.SPRING_STATE)
        except usb.core.USBError:
            print("Could not send TOGGLE_STATE vendor request.")

    def wall_state(self):
        try:
            self.dev.ctrl_transfer(0x40, self.WALL_STATE)
        except usb.core.USBError:
            print("Could not send TOGGLE_STATE vendor request.")

    def damp_state(self):
        try:
            self.dev.ctrl_transfer(0x40, self.DAMP_STATE)
        except usb.core.USBError:
            print("Could not send TOGGLE_STATE vendor request.")

    def texture_state(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TEXTURE_STATE)
        except usb.core.USBError:
            print("Could not send TOGGLE_STATE vendor request.")

    def tune_up(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TUNE_UP)
        except usb.core.USBError:
            print("Could not send TOGGLE_STATE vendor request.")

    def tune_down(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TUNE_DOWN)
        except usb.core.USBError:
            print("Could not send TOGGLE_STATE vendor request.")

    def get_current(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.ENC_READ_REG, 0x3FFF, 0, 2)
        except usb.core.USBError:
            print ("Could not send ENC_READ_REG vendor request.")
        else:
            return (int(ret[0]) + 256 * int(ret[1])) & 0x03FF

if __name__=='__main__':
    old_settings = termios.tcgetattr(sys.stdin)

    state = "wall"
    k = 0.3
    damping = 20
    max_bump_size = 1000

    enc = encodertest()
    start = time.time()
    now = start
    data = np.zeros(1000)
    dcs = np.zeros(1000)
    diffs = np.zeros(1000)
    i = 0




    # For calibrating
try:
    tty.setcbreak(sys.stdin.fileno())
    while enc.read_sw1() == 1:
        time.sleep(0.01)
        angle = enc.get_angle()
        #current = enc.get_current()
        data[i] = angle
        i += 1
        if state == "textured":
            DC = (angle % 500) / max_bump_size
            dcs[i] = DC
        elif state == "damped":
            diff = (data[i] - data[i-1])/0.01
            diffs[i] = diff
            #diffs[i] = (diff*0.75 + diffs[i-1]*0.25)
            dc = diffs[i]/300
            dcs[i] = dc


        #print("Started at:", start)
        #print("Measured at:", angle)
        #difference = angle - 2**13
        #print("Difference:", difference)
        # if difference >= 0:
        #     scale = 1560
        # else:
        #     scale = 1230
        # scale = 0.5*((difference/scale) * 0.5) + 0.5
        # print("Spring_force is:", 0.7*difference/1350, '\n')


        if isData():
            c = sys.stdin.read(1)
            if c == '1':         # x1b is ESC
                enc.spring_state()
                state = "spring"

            if c == '2':         # x1b is ESC
                enc.wall_state()
                state = "wall"
            if c == '3':         # x1b is ESC
                enc.damp_state()
                state = "damped"
            if c == '4':         # x1b is ESC
                enc.texture_state()
                state = "textured"
            if c == '5':         # x1b is ESC
                enc.tune_up()
                print("Tuned UP")
                if state == "spring":
                    k += 0.1
                    print("Spring Constant:", k)
                elif state == "damped":
                    damping += 1
                    print("Damping Parameter:", damping)
                elif state == "textured":
                    max_bump_size -= 1
                    print("Max duty cycle:", 500/max_bump_size)
            if c == '6':         # x1b is ESC
                enc.tune_down()
                print("Tuned DOWN")
                if state == "spring":
                    k -= 0.1
                    print("Spring Constant:", k)
                elif state == "damped":
                    damping -= 1
                    print("Damping Parameter:", damping)
                elif state == "textured":
                    max_bump_size += 1
                    print("Max duty cycle:", 500/max_bump_size)
            print("State:", state)

finally:
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)
    #print("hello")
    now = time.time()
    print("Started at:", start)
    print("Finished:", now)
    np.save("damp-velocity.npy", diffs[0:i+1])
    np.save("damp-dc.npy", dcs[0:i+1])
    np.save("damp-position.npy", data[0:i+1])



    # Spin-down test
    # while (now - start) < 3: #Only runs for 8 seconds to keep data small
    #     data[i] = enc.get_angle()
    #     i += 1
    #     time.sleep(0.01)
    #     now = time.time()
    #
    # print("Started at:", start)
    # print("Finished:", now)
    # data = data[0:i+1]
    #
    # np.save("turn.npy", data)
