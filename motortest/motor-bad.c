/*
** Copyright (c) 2018, Bradley A. Minch
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**     2. Redistributions in binary form must reproduce the above copyright
**        notice, this list of conditions and the following disclaimer in the
**        documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
*/

#include "elecanisms.h"
#include "usb.h"

#define MISO            D1
#define MOSI            D0
#define SCK             D2
#define CSn             D3
#define PWM             D5
#define DIR1            D6
#define nEN1            D7



#define MISO_DIR        D1_DIR
#define MOSI_DIR        D0_DIR
#define SCK_DIR         D2_DIR
#define CSn_DIR         D3_DIR

#define MISO_RP         D1_RP
#define MOSI_RP         D0_RP
#define SCK_RP          D2_RP

#define TOGGLE_LED1         0
#define TOGGLE_LED2         1
#define TOGGLE_LED3         2
#define READ_SW1            3
#define READ_SW2            4
#define READ_SW3            5
#define ENC_READ_REG        6

uint16_t even_parity(uint16_t v) {
    v ^= v >> 8;
    v ^= v >> 4;
    v ^= v >> 2;
    v ^= v >> 1;
    return v & 1;
}

WORD enc_readReg(WORD address) {
    WORD cmd, result;
    uint16_t temp;

    cmd.w = 0x4000 | address.w;         // set 2nd MSB to 1 for a read
    cmd.w |= even_parity(cmd.w) << 15;

    CSn = 0;

    SPI2BUF = (uint16_t)cmd.b[1];
    while (SPI2STATbits.SPIRBF == 0) {}
    temp = SPI2BUF;

    SPI2BUF = (uint16_t)cmd.b[0];
    while (SPI2STATbits.SPIRBF == 0) {}
    temp = SPI2BUF;

    CSn = 1;

    __asm__("nop");     // p.12 of the AS5048 datasheet specifies a minimum
    __asm__("nop");     //   high time of CSn between transmission of 350ns
    __asm__("nop");     //   which is 5.6 Tcy, so do nothing for 6 Tcy.
    __asm__("nop");
    __asm__("nop");
    __asm__("nop");

    CSn = 0;

    SPI2BUF = 0;
    while (SPI2STATbits.SPIRBF == 0) {}
    result.b[1] = (uint8_t)SPI2BUF;

    SPI2BUF = 0;
    while (SPI2STATbits.SPIRBF == 0) {}
    result.b[0] = (uint8_t)SPI2BUF;

    CSn = 1;

    return result;
}

void vendor_requests(void) {
    WORD temp;
    uint16_t i;

    switch (USB_setup.bRequest) {
        case TOGGLE_LED1:
            LED1 = !LED1;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case TOGGLE_LED2:
            LED2 = !LED2;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case TOGGLE_LED3:
            LED3 = !LED3;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case READ_SW1:
            BD[EP0IN].address[0] = SW1 ? 1 : 0;
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case READ_SW2:
            BD[EP0IN].address[0] = SW2 ? 1 : 0;
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case READ_SW3:
            BD[EP0IN].address[0] = SW3 ? 1 : 0;
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case ENC_READ_REG:
            temp = enc_readReg(USB_setup.wValue);
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            BD[EP0IN].bytecount = 2;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}

int16_t main(void) {
    uint8_t *RPOR, *RPINR;

    init_elecanisms();



    // Configure pin D5 to produce a 1-kHz PWM signal with a 25% duty cycle
    // using the OC1 module.
    nEN1 = 1; //Turn on enable?
    D5_DIR = OUT;      // configure D5 to be a digital output
    D5 = 0;            // set D5 low

    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;

    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPOR[D5_RP] = OC1_RP;  // connect the OC1 module output to pin D5
    __builtin_write_OSCCONL(OSCCON | 0x40);

    OC1CON1 = 0x1C06;   // configure OC1 module to use the peripheral
                        //   clock (i.e., FCY, OCTSEL<2:0> = 0b111) and
                        //   and to operate in edge-aligned PWM mode
                        //   (OCM<2:0> = 0b110)
    OC1CON2 = 0x001F;   // configure OC1 module to syncrhonize to itself
                        //   (i.e., OCTRIG = 0 and SYNCSEL<4:0> = 0b11111)

    OC1RS = (uint16_t)(FCY / 1e3 - 1.);     // configure period register to
                                            //   get a frequency of 1kHz
    OC1R = OC1RS >> 2;  // configure duty cycle to 25% (i.e., period / 4)
    OC1TMR = 0;         // set OC1 timer count to 0



    // Configure encoder pins and connect them to SPI2
    CSn_DIR = OUT; CSn = 1;
    SCK_DIR = OUT; SCK = 0;
    MOSI_DIR = OUT; MOSI = 0;
    MISO_DIR = IN;

    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;

    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPINR[MISO2_RP] = MISO_RP;
    RPOR[MOSI_RP] = MOSI2_RP;
    RPOR[SCK_RP] = SCK2OUT_RP;
    __builtin_write_OSCCONL(OSCCON | 0x40);

    SPI2CON1 = 0x003B;              // SPI2 mode = 1, SCK freq = 8 MHz
    SPI2CON2 = 0;
    SPI2STAT = 0x8000;

    USB_setup_vendor_callback = vendor_requests;
    init_usb();

    while (USB_USWSTAT != CONFIG_STATE) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
    }
    while (1) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
    }
}
