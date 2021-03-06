#include "elecanisms.h"
#include "usb.h"
#include <stdio.h>

#define MISO            D1
#define MOSI            D0
#define SCK             D2
#define CSn             D3
#define PWM             D5
#define DIR1            D6

int TGOAL = 8;
int TCURR = 0;


// void __attribute__ ((__interrupt__)) _ADC1Interrupt(void) {
//     IFS0bits.AD1IF = 0;
// }


void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    TCURR += 1;

    if (TCURR == TGOAL){
        D6 = 1;
    }

}

int16_t main(void) {
    uint8_t *RPOR, *RPINR;

    init_elecanisms();


    // AD1PCFG = 0xFFFE;      // Configure A/D port// AN0 input pin is analog
    // AD1CON1 = 0x2202;      // Configure sample clock source
    //                        // and conversion trigger mode.
    //                        // Unsigned Fraction format (FORM<1:0>=10),
    //                        // Manual conversion trigger (SSRC<2:0>=000),
    //                        // Manual start of sampling (ASAM=0),
    //                        // No operation in Idle mode (ADSIDL=1),
    //                        // S/H in Sample (SAMP = 1)
    // AD1CON2 = 0;           // Configure A/D voltage reference
    //                        // and buffer fill modes.
    //                        // Vr+ and Vr- from AVdd and AVss (VCFG<2:0>=000),
    //                        // Inputs are not scanned,
    //                        // Interrupt after every sample
    // AD1CON3 = 0x0100;      // Configure sample time = 1Tad,
    //                        // A/D conversion clock as Tcy
    // AD1CHS  = 0;           // Configure input channels,
    //                        // S/H+ input is AN0,
    //                        // S/H- input is Vr- (AVss).
    // AD1CSSL = 0;           // No inputs are scanned.
    // IFS0bits.AD1IF = 0;    // Clear A/D conversion interrupt.
    //                        //  Configure A/D interrupt priority bits (AD1IP<2:0>) here, if
    //                        //  required. Default priority level is 4.
    // IEC0bits.AD1IE  = 1;   // Enable A/D conversion interrupt
    // AD1CON1bits.ADON = 1;  // Turn on A/D
    // AD1CON1bits.SAMP = 1;  // Start sampling the input
    // Delay();               // Ensure the correct sampling time has elapsed
    //                        // before starting conversion.AD1CON1bits.SAMP = 0;
    //                        // End A/D sampling and start conversion
    //                        //  Example code for A/D ISR:

    T1CON = 0x0030;         // set Timer1 period to 0.5s
    PR1 = 0x7A11;

    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1


    D7_DIR = OUT;
    D6_DIR = OUT;

    D6 = 0;
    D7 = 0; //Apparently you have to set enable low

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

    OC1RS = (uint16_t)(FCY / 1e3 - 1.);     // configure period register to
                                            //   get a frequency of 1kHz
    OC1R = OC1RS;  // configure duty cycle to 100% (i.e., period / 4)
    OC1TMR = 0;         // set OC1 timer count to 0
    while (1) {}
}
