//Luc Pham
//1001918323

//Ethan Sprinkle
//1002002357

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include "adc0.h"
#include "gpio.h"
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

#define MOTOR_MASK 1
#define RED_LED_MASK 2

// PortC masks
#define FREQ_IN_MASK 64

// PortE masks
#define AIN3_MASK 1

//Global Variable
uint32_t speed = 512;
uint32_t frequency = 0;
float raw = 0;
float rpm = 0;
uint32_t tempspeed = 0;

void enableTimers()
{
    // Configure Timer 1 as the time base
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER1_TAILR_R = 40000000;                       // set load value to 40e6 for 50 Hz interrupt rate
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

    // Configure Timer 1 as the time base
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER2_TAILR_R = 8000000;                       // set load value to 40e6 for 50 Hz interrupt rate
    TIMER2_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER2A-16);             // turn-on interrupt 39 (TIMER2A)
}

void setMotor1(uint16_t newspeed) //max speed of 1024
{
    PWM1_2_CMPB_R = newspeed;
}

void timer1Isr()
{
    frequency = WTIMER1_TAV_R;                   // read counter input
    WTIMER1_TAV_R = 0;                           // reset counter for next period
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;           // clear interrupt flag
}

void timer2Isr()
{
    tempspeed = speed;
    setMotor1(0);
    waitMicrosecond(155);
    // Use AIN3 input with N=4 hardware sampling
    // Clear FIR filter taps
    raw = readAdc0Ss3();
    raw = 10+(((raw+0.5) / 4096.0 * 3.3) * -5.7); // 10 (Vss) + raw value + .5 (rounding) / 4096 bits * 3.3 Volts * 5.7 [1 / 10k/(47k+10k)]
    rpm = ((166.82*raw) - 59.328);
    setMotor1(tempspeed);
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;           // clear interrupt flag
}

void wideTimer1Isr()
{

}

void initHW()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R5 | SYSCTL_RCGCGPIO_R1;

    //SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    enablePort(PORTF);
    enablePort(PORTB);
    _delay_cycles(3);


    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;  // enable motor

    // Configure three LEDs
    GPIO_PORTF_DEN_R |= MOTOR_MASK | RED_LED_MASK;
    GPIO_PORTF_AFSEL_R |= MOTOR_MASK | RED_LED_MASK;
    GPIO_PORTF_PCTL_R &= ~(GPIO_PCTL_PF1_M);
    GPIO_PORTF_PCTL_R |= GPIO_PCTL_PF1_M1PWM5;

    // Configure PWM module 1 to drive RGB LED
    // RED   on M1PWM5 (PF1), M1PWM2b
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;                // reset PWM1 module
    SYSCTL_SRPWM_R = 0;                              // leave reset state
    PWM1_2_CTL_R = 0;                                // turn-off PWM1 generator 2 (drives outs 4 and 5)
    PWM1_2_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
                                                     // output 5 on PWM1, gen 2b, cmpb

    PWM1_2_LOAD_R = 1024;                            // set frequency to 40 MHz sys clock / 2 / 1024 = 19.53125 kHz
                                                     // (internal counter counts down from load value to zero)

    PWM1_2_CMPB_R = 0;                               // red off (0=always low, 1023=always high)

    PWM1_2_CTL_R = PWM_1_CTL_ENABLE;                 // turn-on PWM1 generator 2
    PWM1_ENABLE_R = PWM_ENABLE_PWM5EN;

    //CONFIGURE SW2
    setPinCommitControl(PORTF, 0);
    selectPinDigitalInput(PORTF, 0);
    enablePinPullup(PORTF, 0);

    //CONFIGURE SW1
    selectPinDigitalInput(PORTF, 4);
    enablePinPullup(PORTF, 4);

    // Configure SIGNAL_IN for frequency and time measurements
    GPIO_PORTC_AFSEL_R |= FREQ_IN_MASK;              // select alternative functions for SIGNAL_IN pin
    GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC6_M;           // map alt fns to SIGNAL_IN
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC6_WT1CCP0;
    GPIO_PORTC_DEN_R |= FREQ_IN_MASK;                // enable bit 6 for digital input

    // Configure AIN3 as an analog input
    GPIO_PORTE_AFSEL_R |= AIN3_MASK;                 // select alternative functions for AN3 (PE0)
    GPIO_PORTE_DEN_R &= ~AIN3_MASK;                  // turn off digital operation on pin PE0
    GPIO_PORTE_AMSEL_R |= AIN3_MASK;                 // turn on analog operation on pin PE0
}

//lab 6 has freq_time example.
int main(void)
  {
    initHW();
    initUart0();
    initAdc0Ss3();
    setUart0BaudRate(115200, 40e6);
    setMotor1(speed);
    enableTimers();
    setAdc0Ss3Mux(3);
    setAdc0Ss3Log2AverageCount(2);
    char str[50];
    uint32_t tempspeed = 0;
    uint32_t tempfrequency = 0;
    float temprpm;

    while(1)
    {
        putsUart0("test");
        tempspeed = (speed*100)/1024;
        snprintf(str, sizeof(str), "Percent Speed: %2"PRIu32" \n", tempspeed);
        putsUart0(str);
        tempfrequency = ((frequency/32)*60);
        snprintf( str, sizeof(str), "RPM optical: %2"PRIu32"\n", tempfrequency);
        putsUart0(str);
        temprpm = rpm;
        snprintf( str, sizeof(str), "RPM voltage: %f \n\n", temprpm);
        putsUart0(str);

        //IF SW1 IS PRESSED
        if(getPinValue(PORTF, 4) == 0)
        {
            waitMicrosecond(1000000);
            //MAKE SURE IT DOESN'T GO OVER 100% DUTY CYCLE
            if(speed != 1012)
            {
                //INCREASE DUTY CYCLE BY ~10%
                speed = speed + 100;
            }
            setMotor1(speed);
        }
        //IF SW2 IS PRESSED
        if(getPinValue(PORTF, 0) == 0)
        {
            waitMicrosecond(1000000);
            //MAKE SURE IT DOESN'T GO UNDER 0% DUTY CYCLE
            if(speed != 12)
            {
                //DECREASE DUTY CYCLE BY ~10%
                speed = speed - 100;
            }
            setMotor1(speed);
        }

        waitMicrosecond(100000);
    }

	return 0;
}
