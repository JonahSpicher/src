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
int ball_flag = 0;
int celebration_counter = 0;
int slow_timer = 0;

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    if (ball_flag == 1) { //If a ball went through, blink fast
        slow_timer = 0;
        LED1 = !LED1;
        LED2 = !LED2;
        celebration_counter ++;
        if (celebration_counter > 6) { //But dont blink fast for very long
            ball_flag = 0;
            celebration_counter = 0;
        }
    }
    else { //If a ball didn't go through, blink slowly
        if (slow_timer > 4) {
            slow_timer = 0;
            LED1 = !LED1;
            LED2 = !LED2;
        }
        slow_timer ++;
    }

    //LED3 = !LED3;           // make sure this is running
}

int16_t main(void) {
    init_elecanisms();

    T1CON = 0x0020;         // set Timer1 period to ~0.25s
    PR1 = 0x7A11;

    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1



    ball_flag = 0;

    //LED2 = ON;

    //Do waiting behavior
    while (1) {
        int val = read_analog(A0_AN);
        if (ball_flag == 0){
            if (val > 500) {
                ball_flag = 1;
            }
        }
    }
}
