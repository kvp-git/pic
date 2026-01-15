/**
 * @file main.c
 * @author kvp
 * @date 2026-01-15
 * @brief Main function
 */
#include <xc.h> // Include standard Microchip XC8 header

#define _XTAL_FREQ 4000000 // Define the clock frequency (e.g., 4MHz) for the delay function

#pragma config OSC = IntRC     // Internal oscillator
#pragma config WDT = OFF       // Watchdog Timer disabled
#pragma config CP = OFF        // Code Protection off
#pragma config MCLRE = OFF     // GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD

#define LED_NONE    (0)
#define LED_BLUE1   (1) // GPIO0: blue 1  - out 1
#define LED_BLUE2   (2) // GPIO1: blue 2  - out 2
#define LED_WHITE1  (4) // GPIO2: white 1 - out 3
#define KEY_BUTTON  (8) // GPIO3: input
#define LED_WHITE2 (16) // GPIO4: white 2 - out 5
#define LED_WHITE3 (32) // GPIO5: white 3 - out 4

// 1 tick is 250 msec for 2 flashes / sec on the blue lights
#define WHITE2_SWITCH_COUNT (60*4) // 60 seconds
#define WHITE3_SWITCH_COUNT (40*4) // 40 seconds

uint8_t phase1;
uint8_t phase2;
uint8_t phase3;
uint8_t counter2;
uint8_t counter3;
uint8_t keyPressed;
uint8_t t;
uint8_t out;

int main () // press Ctrl + Shift + B to build
{
    OPTION = 0b11011111; // disable timer T0CS input mode
    TRIS = KEY_BUTTON;   // set input and output pins
    phase1 = 0;
    phase2 = 0;
    phase3 = 0;
    counter2 = WHITE2_SWITCH_COUNT;
    counter3 = WHITE3_SWITCH_COUNT;
    GPIO = LED_BLUE1 | LED_BLUE2 | LED_WHITE1 | LED_WHITE2 | LED_WHITE3;
    __delay_ms(1000);
    GPIO = LED_NONE;
    while (1)
    {
        keyPressed = 0;
        for (t = 0; t < 25; t++) // wait for 250 milliseconds
        {
            if ((GPIO & KEY_BUTTON) == 0)
                keyPressed = 1;
            __delay_ms(10);
        }
        out = 0;
        phase1 = (phase1 + 1) & 1;
        if (phase1 == 0)
            out |= LED_BLUE1;
        else
            out |= LED_BLUE2;
        if (keyPressed == 1)
        {
            out |= LED_WHITE1;
            if (phase2 == 0)
                out |= LED_WHITE2;
            if (phase3 == 0)
                out |= LED_WHITE3;
            if (--counter2 == 0)
            {
                counter2 = WHITE2_SWITCH_COUNT;
                phase2 = (phase2 + 1) & 1;
            }
            if (--counter3 == 0)
            {
                counter3 = WHITE3_SWITCH_COUNT;
                phase3 = (phase3 + 1) & 1;
            }
        } else
        {
            // reset to everything on
            counter2 = WHITE2_SWITCH_COUNT;
            counter3 = WHITE3_SWITCH_COUNT;
            phase2 = 0;
            phase3 = 0;
        }
        GPIO = out;
    }
    return 0;
}
