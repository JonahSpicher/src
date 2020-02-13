
import usb.core
import time
import numpy as np

class encodertest:

    def __init__(self):
        self.TOGGLE_LED1 = 0
        self.TOGGLE_LED2 = 1
        self.TOGGLE_LED3 = 2
        self.READ_SW1 = 3
        self.READ_SW2 = 4
        self.READ_SW3 = 5
        self.ENC_READ_REG = 6
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

if __name__=='__main__':
    enc = encodertest()
    start = time.time()
    now = start
    data = np.zeros(1000)
    i = 0

    # For calibrating
    # while enc.read_sw1() == 1:
    #     print(enc.get_angle())
    #     time.sleep(0.5)

    # Spin-down test
    while (now - start) < 8: #Only runs for 8 seconds to keep data small 
        data[i] = enc.get_angle()
        i += 1
        time.sleep(0.01)
        now = time.time()

    print("Started at:", start)
    print("Finished:", now)
    data = data[0:i+1]

    np.save("spindown.npy", data)
