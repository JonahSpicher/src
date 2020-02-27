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
#include "math.h"
#include <stdio.h>

#define ENC_MOSI        D0
#define ENC_MISO        D1
#define ENC_SCK         D2
#define ENC_CSn         D3
#define PWM             D5
#define DIR1            D6
#define M_EN            D7

#define ENC_MISO_DIR        D1_DIR
#define ENC_MOSI_DIR        D0_DIR
#define ENC_SCK_DIR         D2_DIR
#define ENC_CSn_DIR         D3_DIR

#define ENC_MISO_RP         D1_RP
#define ENC_MOSI_RP         D0_RP
#define ENC_SCK_RP          D2_RP

#define SPRING_MODE         0
#define WALL_MODE           1
#define DAMP_MODE           2
#define TEXTURE_MODE        3
#define SWITCH_1            4
#define ENC_READ_REG        6

int TSTART = 4;     //Allow for time to connect to the board with python
int TSPINUP = 4;    //Allow motor to get up to speed
int TCURR = 0;      //Keep track of how many times the timer has been triggered
float K = 3;      //spring constant (maximum duty cycle of resistance)
int MSTATE = 1;     //Toggles behavior of motor

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    TCURR += 1;
    LED1 = !LED1; //Toggle LED to show things are running
    //Disables motor if its before or after spinup
}

int createMask(unsigned a, unsigned b)
{
   int r = 0;
   int i;
   for (i=a; i<=b; i++)
       r |= 1 << i;

   return r;
}

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

    ENC_CSn = 0;

    SPI2BUF = (uint16_t)cmd.b[1];
    while (SPI2STATbits.SPIRBF == 0) {}
    temp = SPI2BUF;

    SPI2BUF = (uint16_t)cmd.b[0];
    while (SPI2STATbits.SPIRBF == 0) {}
    temp = SPI2BUF;

    ENC_CSn = 1;

    __asm__("nop");     // p.12 of the AS5048 datasheet specifies a minimum
    __asm__("nop");     //   high time of CSn between transmission of 350ns
    __asm__("nop");     //   which is 5.6 Tcy, so do nothing for 6 Tcy.
    __asm__("nop");
    __asm__("nop");
    __asm__("nop");

    ENC_CSn = 0;

    SPI2BUF = 0;
    while (SPI2STATbits.SPIRBF == 0) {}
    result.b[1] = (uint8_t)SPI2BUF;

    SPI2BUF = 0;
    while (SPI2STATbits.SPIRBF == 0) {}
    result.b[0] = (uint8_t)SPI2BUF;

    ENC_CSn = 1;

    return result;
}

void vendor_requests(void) {
    WORD temp;
    uint16_t i;

    switch (USB_setup.bRequest) {
        case SPRING_MODE:
            MSTATE = 0;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case WALL_MODE:
            MSTATE = 1;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case DAMP_MODE:
            MSTATE = 2;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case TEXTURE_MODE:
            MSTATE = 3;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case SWITCH_1:
            BD[EP0IN].address[0] = SW1 ? 1 : 0;
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

    T1CON = 0x0030;         // set Timer1 period to 0.5s
    PR1 = 0x7A11;

    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1

    D7_DIR = OUT;
    D6_DIR = OUT;

    LED2 = 0;
    LED3 = 0;

    DIR1 = 1; //Initial motor direction I guess
    M_EN = 1; //Initially disable

    // Configure pin D5 to produce a 1-kHz PWM signal with a 25% duty cycle
    // using the OC1 module.
    D5_DIR = OUT;      // configure D5 to be a digital output
    PWM = 0;            // set D5 low

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

    OC1RS = (uint16_t)(FCY / 30e3 - 1.);     // configure period register to
                                            //   get a frequency of 1kHz
    OC1R = OC1RS;  // configure duty cycle to 100%
    OC1TMR = 0;         // set OC1 timer count to 0

    // Configure encoder pins and connect them to SPI2
    ENC_CSn_DIR = OUT; ENC_CSn = 1;
    ENC_SCK_DIR = OUT; ENC_SCK = 0;
    ENC_MOSI_DIR = OUT; ENC_MOSI = 0;
    ENC_MISO_DIR = IN;

    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;

    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPINR[MISO2_RP] = ENC_MISO_RP;
    RPOR[ENC_MOSI_RP] = MOSI2_RP;
    RPOR[ENC_SCK_RP] = SCK2OUT_RP;
    __builtin_write_OSCCONL(OSCCON | 0x40);

    SPI2CON1 = 0x003B;              // SPI2 mode = 1, SCK freq = 8 MHz
    SPI2CON2 = 0;
    SPI2STAT = 0x8000;

    USB_setup_vendor_callback = vendor_requests;
    init_usb();
    int mask = createMask(0, 13);

    while (USB_USWSTAT != CONFIG_STATE) {
        #ifndef USB_INTERRUPT
            usb_service();
        #endif
    }
    while (1) {
        #ifndef USB_INTERRUPT
            usb_service();
        #endif
        disable_interrupts();
        float val = (float)(mask & enc_readReg((WORD)0x3FFF).w);
        int prev_val = 8192;
        int prev_dif = 0;
        enable_interrupts();
        switch (MSTATE) {
            case 0: ; //Spring
                LED2 = 1;
                LED3 = 1;
                float difference = val-pow(2,13);
                float scale;
                if (difference >= 0) {
                    DIR1 = 0;
                    scale = 1560;
                }
                else {
                    DIR1 = 1;
                    scale = 1230;
                }
                //float spring_force = (8*((abs(difference) / scale) * K) + 2)/10; // Offset of 0.2, then linear for the remaining 0.8
                OC1R = OC1RS * (8*((abs(difference) / scale) * 3)/10 + 2)/10; //K is 3/10
                M_EN = 0;
                break;

            case 1: //Wall
                OC1R = OC1RS>>2;
                DIR1 = 0;
                if (val > 8500) {
                    LED3 = 1;
                    M_EN = 0; //Enables once past a certain point
                }
                else {
                    LED3 = 0;
                    M_EN = 1; //Otherwise disables
                }
                break;
            case 2: ; //damping
                LED2 = 1;
                LED3 = 0;
                int d_dif = (int)(val - prev_val);
                //Set direction based on sign
                if (d_dif < 0){
                    DIR1 = 0;
                }
                else {
                    DIR1 = 1;
                }
                d_dif = ((prev_dif>>2) + (3*d_dif>>2)); // 1/4 old values, 3/4 new values
                int max_dif = 3;
                int d_scale = 20; //because it moves very fast
                //float damp_force = difference/max_dif;
                OC1R = d_scale*(OC1RS*d_dif)/max_dif;
                prev_dif = d_dif;
                prev_val = val;
                M_EN = 0;
                break;
            case 3: ; //texture
                LED2 = 0;
                LED3 = 0;
                DIR1 = 0;
                int dist = (int)val % 20; // Should feel bumpy as you pass multiples of twenty
                OC1R = (OC1RS*dist)/(40); // Scales to a max of 50% duty cycle
                M_EN = 1;
                break;
        }
    }

}
