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
#include <stdlib.h>
#include <stdio.h>
int ball_flag = 0;
int return_counter = 0;
int started = 0;
int game_timer = 0;
int end_game = 0;
int won_flag = 0;
int lost_flag = 0;
int slow_timer = 0;
int score = 0;
uint16_t servo_offset, servo_multiplier;
WORD32 servo_temp;

#define SERVO_MIN_WIDTH     900e-6
#define SERVO_MAX_WIDTH     2.1e-3

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;      // lower Timer1 interrupt flag
    //LED1 = !LED1;
    if (won_flag == 1){
        D0 = !D0; //Ugly blocks like this are to prevent pin setting from being ignored
        int a = 1;
        D1 = !D1;
        a = 1;
        D2 = !D2;
        a = 1;
        D3 = !D3;
        a = 1;
        D4 = !D4;
        a = 1;
        D5 = !D5;
        a = 1;
        D6 = !D6;
        a = 1;
        D7 = !D7;
        a = 1;
        D8 = !D8;
        a = 1;
        D9 = !D9;
    }
    else if (lost_flag == 1) {
        slow_timer ++;
        if (slow_timer == 4){
            slow_timer = 0;
            D0 = !D0;
            int a = 1;
            D1 = !D1;
        }
    }
    if (started == 1){
        game_timer ++;
        if (game_timer > 960) { //Sets a timer for 2 minutes
            started = 0;
            //set end game flag
            end_game = 1;
        }
        if (ball_flag == 1) { //If a ball went through, return a ball
            return_counter ++;
            //Move servo to let ball through
            servo_temp.ul = 0;
            OC1RS = servo_offset + servo_temp.w[1];

            if (return_counter > 6) { // Run at end of celebration period
                ball_flag = 0; //ends de-bouncing
                return_counter = 0;

                //Close servo to ready next ball
                servo_temp.ul = 65535 * (uint32_t)servo_multiplier;
                OC1RS = servo_offset + servo_temp.w[1];
            }
        }
        if (score == 1) {D0 = !D0;}
        else if (score == 2) {D0 = 1; int a = 1; D1 = !D1;}
        else if (score == 3) {D0 = 1; int a = 1; D1 = 1; a = 1; D2 = !D2;}
        else if (score == 4) {D0 = 1; int a = 1; D1 = 1; a = 1; D2 = 1; a = 1; D3 = !D3;}
        else if (score == 5) {D0 = 1; int a = 1; D1 = 1; a = 1; D2 = 1; a = 1; D3 = 1; a = 1; D4 = !D4;}
        else if (score == 6) {D0 = 1; int a = 1; D1 = 1; a = 1; D2 = 1; a = 1; D3 = 1; a = 1; D4 = 1; a = 1; D5 = !D5;}
        else if (score == 7) {D0 = 1; int a = 1; D1 = 1; a = 1; D2 = 1; a = 1; D3 = 1; a = 1; D4 = 1; a = 1; D5 = 1; a = 1; D6 = !D6;}
        else if (score == 8) {D0 = 1; int a = 1; D1 = 1; a = 1; D2 = 1; a = 1; D3 = 1; a = 1; D4 = 1; a = 1; D5 = 1; a = 1; D6 = 1; a = 1; D7 = !D7;}
        else if (score == 9) {D0 = 1; int a = 1; D1 = 1; a = 1; D2 = 1; a = 1; D3 = 1; a = 1; D4 = 1; a = 1; D5 = 1; a = 1; D6 = 1; a = 1; D7 = 1; a = 1; D8 = !D8;}
        else if (score == 10){D0 = 1; int a = 1; D1 = 1; a = 1; D2 = 1; a = 1; D3 = 1; a = 1; D4 = 1; a = 1; D5 = 1; a = 1; D6 = 1; a = 1; D7 = 1; a = 1; D8 = 1; a = 1; D9 = 1;}


    }

    //LED3 = !LED3;           // make sure this is running
}



int16_t main(void) {
    init_elecanisms();
    D0_DIR = OUT; //Score LED's
    D1_DIR = OUT;
    D2_DIR = OUT;
    D3_DIR = OUT;
    D4_DIR = OUT;
    D5_DIR = OUT;
    D6_DIR = OUT;
    D7_DIR = OUT;
    D8_DIR = OUT;
    D9_DIR = OUT;


    D11_DIR = OUT; // For neopixels, if that happens
    D12_DIR = IN;  // Start button input
    D13_DIR = OUT; // Servo pulse

    T2CON = 0x0020;         // set Timer2 period to ~0.25s
    PR2 = 0x7A11;
    TMR2 = 0;               // set Timer2 count to 0
    IFS0bits.T2IF = 0;      // lower Timer2 interrupt flag
    IEC0bits.T2IE = 1;      // enable Timer2 interrupt
    T2CONbits.TON = 1;      // turn on Timer2

    ball_flag = 0;

    //Servo chunk
    uint8_t *RPOR, *RPINR;
    servo_offset = (uint16_t)(FCY * SERVO_MIN_WIDTH);
    servo_multiplier = (uint16_t)(FCY * (SERVO_MAX_WIDTH - SERVO_MIN_WIDTH));
    D13 = 0;
    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;
    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPOR[D13_RP] = OC1_RP;  // connect the OC1 module output to pin D13
    __builtin_write_OSCCONL(OSCCON | 0x40);
    OC1CON1 = 0x1C0F;   // configure OC1 module to use the peripheral
                        //   clock (i.e., FCY, OCTSEL<2:0> = 0b111),
                        //   TRIGSTAT = 1, and to operate in center-aligned
                        //   PWM mode (OCM<2:0> = 0b111)
    OC1CON2 = 0x008B;   // configure OC1 module to trigger from Timer1
                        //   (OCTRIG = 1 and SYNCSEL<4:0> = 0b01011)
    // set OC1 pulse width to 1.5ms (i.e. halfway between 0.9ms and 2.1ms)
    servo_temp.ul = 65535 * (uint32_t)servo_multiplier;
    OC1RS = servo_offset + servo_temp.w[1];
    OC1R = 1;
    OC1TMR = 0;
    T1CON = 0x0010;     // configure Timer1 to have a period of 20ms
    PR1 = 0x9C3F;
    TMR1 = 0;
    T1CONbits.TON = 1;



    while(!D12) {
        //do nothing until the button is pressed
    }

    //After button is pressed:
    started = 1; //Tell the timer loop the game has started
    ball_flag = 1; //Return a ball


    while (1) {
        if (score == 10) {end_game = 1;} // If the score is ten, the game is over
        if (end_game == 1){ //if the game is over, figure out why
            started = 0;
            if (score == 10){
                //Do winning celebration

                won_flag = 1; // If we won, start the win celebration
                D0 = 0;
            }
            else {
                //do losing celebration
                lost_flag = 1;
                D0 = 1;
                int a = 1;
                D1 = 0;
                a = 1;
                D3 = 0;
                a = 1;
                D5 = 0;
                a = 1;
                D7 = 0;
                a = 1;
                D9 = 0;
            }
            D2 = 0;
            int a = 1;
            D4 = 0;
            a = 1;
            D6 = 0;
            a = 1;
            D8 = 0;

            while(!D12){}
            //Now reset
            D0 = 0;
            started = 1;
            D1 = 0;
            score = 0;
            D2 = 0;
            ball_flag = 1;
            D3 = 0;
            game_timer = 0;
            D4 = 0;
            end_game = 0;
            D5 = 0;
            won_flag = 0;
            D6 = 0;
            lost_flag = 0;
            D7 = 0;
            a = 1;
            D8 = 0;
            a = 1;
            D9 = 0;

        }

        //Check for a score
        if (ball_flag == 0){
            //These ifs happen once per score, timer debounces
            if (read_analog(A0_AN) > 500) { //Score in hole 1
                //LED1 = !LED1;
                ball_flag = 1;
                score += 1;
            }
            if (read_analog(A1_AN) > 500) { //Score in hole 2
                //LED1 = !LED1;
                ball_flag = 1;
                score += 2;
            }
            if (read_analog(A2_AN) > 500) { //Score in hole 3
                //LED1 = !LED1;
                ball_flag = 1;
                score += 1;
            }
            if (read_analog(A3_AN) > 500) { //Score in hole 4
                //LED1 = !LED1;
                ball_flag = 1;
                score += 2;
            }
            if (read_analog(A4_AN) > 500) { //You missed
                ball_flag = 1;
            }
        }

    }
}
