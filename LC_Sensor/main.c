//Luc Pham
//Ethan Sprinkle

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "gpio.h"
#include "wait.h"
#include "uart0.h"
#include "clock.h"
#include "tm4c123gh6pm.h"

#define CO_ PORTC,7
#define timerPin PORTC,6

#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))
#define BLUE_LED     (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))

// PortC masks
#define FREQ_IN_MASK 64
#define BLUE_LED_MASK 4
#define GREEN_LED_MASK 8
#define PUSH_BUTTON_MASK 16

uint32_t frequency = 0;


void enableTimerMode()
{
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER1_TAMR_R = TIMER_TAMR_TACDIR | TIMER_TAMR_TAMR_1_SHOT;
                                                     // configure for edge time mode, count up
    WTIMER1_CTL_R = TIMER_CTL_TAEVENT_POS;           // measure time from positive edge to positive edge
    WTIMER1_IMR_R = TIMER_IMR_CAEIM;                 // turn-on interrupts
    WTIMER1_TAV_R = 0;                               // zero counter for first period
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
    NVIC_EN3_R = 1 << (INT_WTIMER1A-16-96);         // turn-on interrupt 112 (WTIMER1A)
}

void disableTimerMode()
{
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter
    NVIC_DIS3_R = 1 << (INT_WTIMER1A-16-96);        // turn-off interrupt 112 (WTIMER1A)
}

void enableCounterMode()
{
    // Configure Timer 1 as the time base
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER1_TAILR_R = 40000000;                       // set load value to 40e6 for 1 Hz interrupt rate
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER1A-16);             // turn-on interrupt 37 (TIMER1A)

    // Configure Wide Timer 1 as counter of external events on CCP0 pin
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER1_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER1_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge count mode, count up
    WTIMER1_CTL_R = TIMER_CTL_TAEVENT_POS;           // count positive edges
    WTIMER1_IMR_R = 0;                               // turn-off interrupts
    WTIMER1_TAV_R = 0;                               // zero counter for first period
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
}

void disableCounterMode()
{
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off time base timer
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off event counter
    NVIC_DIS0_R = 1 << (INT_TIMER1A-16);            // turn-off interrupt 37 (TIMER1A)
}

void timer1Isr()
{
    frequency = WTIMER1_TAV_R;                   // read counter input
    WTIMER1_TAV_R = 0;                           // reset counter for next period
    //GREEN_LED ^= 1;                              // status
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;           // clear interrupt flag
}

void initHw()
{
    initSystemClockTo40Mhz();
    SYSCTL_RCGCACMP_R |= SYSCTL_RCGCACMP_R0;
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    enablePort(PORTC);
    COMP_ACREFCTL_R |= COMP_ACREFCTL_EN;
    COMP_ACREFCTL_R |= 0x0000000F;
    COMP_ACCTL0_R |=  0x0000040C; //enables TOEN and ISEN
    COMP_ACCTL0_R |= COMP_ACCTL0_CINV;
    COMP_ACCTL0_R |= COMP_ACCTL0_ASRCP_REF;
    waitMicrosecond(10);

    // Configure LED and pushbutton pins
    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | BLUE_LED_MASK;  // bits 1 and 2 are outputs, other pins are inputs
    GPIO_PORTF_DIR_R &= ~PUSH_BUTTON_MASK;               // bit 4 is an input
    GPIO_PORTF_DR2R_R |= GREEN_LED_MASK | BLUE_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= PUSH_BUTTON_MASK | GREEN_LED_MASK | BLUE_LED_MASK;

    // Configure SIGNAL_IN for frequency and time measurements
    GPIO_PORTC_AFSEL_R |= FREQ_IN_MASK;              // select alternative functions for SIGNAL_IN pin
    GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC6_M;           // map alt fns to SIGNAL_IN
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC6_WT1CCP0;
    GPIO_PORTC_DEN_R |= FREQ_IN_MASK;                // enable bit 6 for digital input

    //GPIO_PCTL_PC7_WT1CCP1;
}
int main(void)
{
    initHw();
    initUart0();
    setUart0BaudRate(115200, 40e6);
    enableCounterMode();

    selectPinPushPullOutput(PORTC, 4);
    float timeVal;
    float capacitance;
    float volume;
    char line[40];

    char str[40];
    while (true)
    {
        sprintf(str, "Frequency: %7"PRIu32" (Hz)\n", frequency);
        putsUart0(str);

        if(frequency >= 120000)
        {
            GREEN_LED = 1;
            BLUE_LED = 0;
        }
        else if(frequency >= 97000 && frequency <= 120000)
        {
            BLUE_LED = 1;
            GREEN_LED = 0;
        }
        else
        {
            BLUE_LED = 0;
            GREEN_LED = 0;
        }
        waitMicrosecond(100000);

    }
    /*
    while(true)
    {
        setPinValue(PORTC, 4, 1);
        waitMicrosecond(1000000);
        setPinValue(PORTC, 4, 0);

        enableTimerMode();

        while(COMP_ACSTAT0_R != 2);

        timeVal = WTIMER1_TAV_R;
        disableTimerMode();
        sprintf(line, "value of time: %f us \n", timeVal/40);
        putsUart0(line);
        capacitance = ((timeVal/40000000) / 0.1379047953);
        sprintf(line, "value of capacitance: %f uF \n", capacitance);
        putsUart0(line);
        float timetemp = timeVal/40;
        volume = (74.809*timetemp) - 683.42;
        sprintf(line, "value of volume: %f mL \n\n", volume);
        putsUart0(line);
    }
    */

	return 0;
}
