/**
 * @file main.c
 * @author kvp
 * @date 2025-12-14
 * @brief Main function
 */
#include <xc.h> // Include standard Microchip XC8 header

#define _XTAL_FREQ 4000000 // Define the clock frequency (e.g., 4MHz) for the delay function

#pragma config OSC = IntRC     // Internal oscillator
#pragma config WDT = OFF       // Watchdog Timer disabled
#pragma config CP = OFF        // Code Protection off
#pragma config MCLRE = OFF     // GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD

#define LED_NONE    (0)
#define LED_RED1    (1)
#define LED_RED2    (2)
#define LED_WHITE   (4)
#define KEY_TRACK   (8)
#define KEY_BUTTON (16)

#define RED_DELAY_COUNT  (60) // 60 = 30 seconds, 120 = 60 seconds, 240 = 120 seconds
#define LONGPRESS_COUNT   (5) // 2.5 seconds

uint8_t state;
uint8_t phase;
uint8_t timer;
uint8_t longpress;
uint8_t keyPressed;
uint8_t sensorActive;
uint8_t t;

enum STATES
{
    STATE_WHITE,
    STATE_RED,
    STATE_FIXRED1, // waiting for button release
    STATE_FIXRED2, // waiting for button press again
    STATE_FIXRED3, // waiting for button release again
};

int main () // press Ctrl + Shift + B to build
{
    OPTION = 0b11011111; // disable timer T0CS input mode
    TRIS = KEY_TRACK | KEY_BUTTON; // set input and output pins
    state = STATE_RED;
    phase = 0;
    timer = RED_DELAY_COUNT;
    longpress = 0;
    GPIO = LED_RED1 | LED_RED2 | LED_WHITE;
    __delay_ms(1000);
    GPIO = LED_NONE;
    while (1)
    {
        if (keyPressed != 0)
        {
            if(longpress < 255)
                longpress++;
        } else
        {
            longpress = 0;
        }
        switch (state)
        {
            case STATE_WHITE:
                if ((sensorActive != 0) || (keyPressed != 0))
                {
                    state = STATE_RED;
                    timer = RED_DELAY_COUNT;
                }
                break;
            case STATE_RED:
                if ((sensorActive != 0) || (keyPressed != 0))
                    timer = RED_DELAY_COUNT;
                if (longpress >= LONGPRESS_COUNT)
                    state = STATE_FIXRED1;
                break;
            case STATE_FIXRED1:
                timer = RED_DELAY_COUNT;
                if (keyPressed == 0)
                    state = STATE_FIXRED2;
                break;
            case STATE_FIXRED2:
                timer = RED_DELAY_COUNT;
                if (keyPressed != 0)
                    state = STATE_FIXRED3;
                break;
            case STATE_FIXRED3:
                timer = RED_DELAY_COUNT;
                if (keyPressed == 0)
                {
                    state = STATE_WHITE;
                    timer = 0;
                }
                break;
        }
        if (timer != 0)
        {
            timer--;
            if (timer == 0)
                state = STATE_WHITE;
        }
        if (state == STATE_WHITE)
        {
            if (phase == 0)
                GPIO = LED_WHITE;
            else
                GPIO = LED_NONE;
        } else
        {
            if (phase == 0)
                GPIO = LED_RED1;
            else
                GPIO = LED_RED2;
        }
        keyPressed = 0;
        sensorActive = 0;
        for (t = 0; t < 50; t++) // wait for 500 milliseconds
        {
            if ((GPIO & KEY_BUTTON) == 0)
                keyPressed = 1;
            if ((GPIO & KEY_TRACK) == 0)
                sensorActive = 1;
            __delay_ms(10);
        }
        phase = (phase + 1) & 1;
    }
    return 0;
}
