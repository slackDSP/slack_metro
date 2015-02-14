/*
 * File:   slack_lfo_main.c
 * Author: Ian Maltby
 *
 * Created on January 8th, 2015, 7:10 PM
 *                 __________
 *          VDD ---|         |--- VSS
 *      MIDI In ---| 12F1822 |--- Red LED
 *   3/4 switch ---|         |--- Green LED
 *              ---|_________|---
 */

#include <xc.h>

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

#define _XTAL_FREQ 32000000  //processor speed for delay macro
#define leds LATA
#define red 1
#define green 2
#define both 3
#define tick 248
#define start 250
#define stop 252
#define cont 251
#define switch34 RA4
#define pulse_switch RA3

volatile char tock = 0;
volatile char beat = 0;
volatile char run = 0;

void main() {

    OSCCONbits.IRCF = 14; //8Mhz clock * 4x PLL = 32Mhz
    CM1CON0bits.C1ON = 0; //disable comparator
    RXDTSEL = 1; //RX on RA5 (pin2)
    CREN = 1; //Enable RX
    SYNC = 0;
    SPEN = 1;
    BRG16 = 0;
    BRGH = 0;
    SPBRG = 15; //31250hz Baud rate
    RCIE = 1;
    PEIE = 1;
    GIE = 1;
    ANSELA = 0; //All pins digital
    TRISA0 = 0;
    TRISA1 = 0;
    TRISA2 = 0;
    TRISA4 = 1;
    leds = 0;

    for (;;) {

        while (!(run && tock));

        if (beat == 0)
            leds = both | 4;
        else
            leds = green | (~pulse_switch << 2);

        __delay_ms(10);
        leds = leds & 3;
        __delay_ms(90);
        leds = 0;

        beat++;
        if (beat > (2 + switch34))
            beat = 0;
        tock = 0;
    }

}

void interrupt isr(void) {

    unsigned char midi_in = 0;
    static char tick_counter = 0;

    midi_in = RCREG;

    if (midi_in == tick) {
        tick_counter++;
        if (tick_counter > 23) {
            tock = 1;
            tick_counter = 0;

        }
    }

    if (midi_in == start) {
        run = 1;
        beat = 0;
        tick_counter = 23;
    }

    if (midi_in == cont) {
        run = 1;
        tick_counter = 23;
    }

    if (midi_in == stop) {
        run = 0;
    }

}
