//Luc Pham
//1001918323

//Ethan Sprinkle
//1002002357

#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "clock.h"
#include "wait.h"
#include "uart0.h"

#define DIRA PORTE,0
#define DIRB PORTE,1
#define DIRC PORTE,2
#define DIRD PORTE,3

#define OPTICALSENS PORTF,4

// Useless stuff

#define MOTOR_MASK 1
#define RED_LED_MASK 2
#define BLUE_LED_MASK 4
#define GREEN_LED_MASK 8


#define MAX_CHARS 80
#define MAX_FIELDS 6

//Keeps track of current step out of 128
int16_t globalstep = 0;
int16_t position = 0;

//sin array of 128 values
float sinarray[128] =
{0.0000, 0.0491, 0.0980, 0.1467, 0.1951, 0.2430, 0.2903, 0.3369, 0.3827, 0.4276, 0.4714, 0.5141, 0.5556, 0.5957, 0.6344, 0.6716, 0.7071, 0.7410, 0.7730, 0.8032, 0.8315, 0.8577, 0.8819, 0.9040, 0.9239, 0.9415, 0.9569, 0.9700, 0.9808, 0.9892, 0.9952, 0.9988,
 1.0000, 0.9988, 0.9952, 0.9892, 0.9808, 0.9700, 0.9569, 0.9415, 0.9236, 0.9040, 0.8816, 0.8577, 0.8315, 0.8032, 0.7730, 0.7410, 0.7071, 0.6716, 0.6344, 0.5957, 0.5556, 0.5141, 0.4714, 0.4276, 0.3827, 0.3369, 0.2903, 0.2430, 0.1951, 0.1467, 0.0980, 0.0491,
 0.0000,-0.0491,-0.0980,-0.1467,-0.1951,-0.2430,-0.2903,-0.3369,-0.3827,-0.4276,-0.4714,-0.5141,-0.5556,-0.5957,-0.6344,-0.6716,-0.7071,-0.7410,-0.7730,-0.8032,-0.8315,-0.8577,-0.8819,-0.9040,-0.9239,-0.9415,-0.9569,-0.9700,-0.9808,-0.9892,-0.9952,-0.9988,
-1.0000,-0.9988,-0.9952,-0.9892,-0.9808,-0.9700,-0.9569,-0.9415,-0.9236,-0.9040,-0.8816,-0.8577,-0.8315,-0.8032,-0.7730,-0.7410,-0.7071,-0.6716,-0.6344,-0.5957,-0.5556,-0.5141,-0.4714,-0.4276,-0.3827,-0.3369,-0.2903,-0.2430,-0.1951,-0.1467,-0.0980,-0.0491};

float cosarray[128] =
{1.0000, 0.9988, 0.9952, 0.9892, 0.9808, 0.9700, 0.9569, 0.9415, 0.9236, 0.9040, 0.8816, 0.8577, 0.8315, 0.8032, 0.7730, 0.7410, 0.7071, 0.6716, 0.6344, 0.5957, 0.5556, 0.5141, 0.4714, 0.4276, 0.3827, 0.3369, 0.2903, 0.2430, 0.1951, 0.1467, 0.0980, 0.0491,
 0.0000,-0.0491,-0.0980,-0.1467,-0.1951,-0.2430,-0.2903,-0.3369,-0.3827,-0.4276,-0.4714,-0.5141,-0.5556,-0.5957,-0.6344,-0.6716,-0.7071,-0.7410,-0.7730,-0.8032,-0.8315,-0.8577,-0.8819,-0.9040,-0.9239,-0.9415,-0.9569,-0.9700,-0.9808,-0.9892,-0.9952,-0.9988,
-1.0000,-0.9988,-0.9952,-0.9892,-0.9808,-0.9700,-0.9569,-0.9415,-0.9236,-0.9040,-0.8816,-0.8577,-0.8315,-0.8032,-0.7730,-0.7410,-0.7071,-0.6716,-0.6344,-0.5957,-0.5556,-0.5141,-0.4714,-0.4276,-0.3827,-0.3369,-0.2903,-0.2430,-0.1951,-0.1467,-0.0980,-0.0491,
 0.0000, 0.0491, 0.0980, 0.1467, 0.1951, 0.2430, 0.2903, 0.3369, 0.3827, 0.4276, 0.4714, 0.5141, 0.5556, 0.5957, 0.6344, 0.6716, 0.7071, 0.7410, 0.7730, 0.8032, 0.8315, 0.8577, 0.8819, 0.9040, 0.9239, 0.9415, 0.9569, 0.9700, 0.9808, 0.9892, 0.9952, 0.9988};

typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
}USER_DATA;

void setMotor1(uint16_t speed) //max speed of 1024
{
    PWM1_2_CMPB_R = speed;
}

void setMotor2(uint16_t speed) //max speed of 1024
{
    PWM1_3_CMPA_R = speed;
}

void getsUart0(USER_DATA *data)
    {
        int count = 0;
        char c;

        while( count < MAX_CHARS)
        {
            c = getcUart0();

            if(c == 127 || c == 8 && count > 0)
            {
                if(count > 0)
                {
                    count--;
                }
            }
            else if(c == 13 || c == 10)
            {
                data->buffer[count++] = '\0';
                return;
            }
            else if(c >= 32)
            {
                data->buffer[count] = c;
                count= count + 1;

                if(count == MAX_CHARS)
                {
                    data->buffer[count] = '\0';
                    return;
                }
            }
        }
    }

void resetBar()
{
        uint8_t i = 0;
        uint8_t j = 0;
        setMotor1(0);
        setMotor2(0);

        while(getPinValue(OPTICALSENS) == false)
        {
            switch(j)
                    {
                    case 0:
                        setMotor2(1023);
                        setPinValue(DIRA, 1);
                        setPinValue(DIRB, 0);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 0);
                        setMotor1(1023);
                        waitMicrosecond(1000000/4);
                        j++;
                        break;
                    case 1:
                        setMotor1(1023);
                        setPinValue(DIRC, 1);
                        setPinValue(DIRD, 0);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 0);
                        setMotor2(1023);
                        waitMicrosecond(1000000/4);
                        j++;
                        break;
                    case 2:
                        setMotor2(1023);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 1);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 0);
                        setMotor1(1023);
                        waitMicrosecond(1000000/4);
                        j++;
                        break;
                    case 3:
                        setMotor1(1023);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 1);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 0);
                        setMotor2(1023);
                        waitMicrosecond(1000000/4);
                        j = 0;
                        break;
                    }
        }

        waitMicrosecond(1000000);
        setMotor1(0);
        setMotor2(0);
        setPinValue(DIRA, 0);
        setPinValue(DIRB, 0);
        setPinValue(DIRC, 0);
        setPinValue(DIRD, 0);
        j = 3;
        i = 0;

        for(i = 0; i < 19; i++)
        {
            switch(j)
                    {
                    case 0:
                        setMotor2(1023);
                        setPinValue(DIRA, 1);
                        setPinValue(DIRB, 0);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 0);
                        setMotor1(1023);
                        waitMicrosecond(1000000/4);
                        j = 3;
                        break;
                    case 1:
                        setMotor1(1023);
                        setPinValue(DIRC, 1);
                        setPinValue(DIRD, 0);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 0);
                        setMotor2(1023);
                        waitMicrosecond(1000000/4);
                        j--;
                        break;
                    case 2:
                        setMotor2(1023);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 1);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 0);
                        setMotor1(1023);
                        waitMicrosecond(1000000/4);
                        j--;
                        break;
                    case 3:
                        setMotor1(1023);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 1);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 0);
                        setMotor2(1023);
                        waitMicrosecond(1000000/4);
                        j--;
                        break;
                    }
        }
}

void turnBar(int steps)
{
    uint16_t i = 0;
    uint16_t j = 0;
    setMotor1(0);
    setMotor2(0);

    //postive number of steps means clockwise motion
    if(steps > 0)
    {
        putsUart0("Clockwise\n");

        for(i = 0; i < steps; i++)
        {
            switch(j)
                    {
                    case 0:
                        setMotor2(1023);
                        setPinValue(DIRA, 1);
                        setPinValue(DIRB, 0);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 0);
                        setMotor1(1023);
                        waitMicrosecond(1000000/4);
                        j++;
                        break;
                    case 1:
                        setMotor1(1023);
                        setPinValue(DIRC, 1);
                        setPinValue(DIRD, 0);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 0);
                        setMotor2(1023);
                        waitMicrosecond(1000000/4);
                        j++;
                        break;
                    case 2:
                        setMotor2(1023);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 1);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 0);
                        setMotor1(1023);
                        waitMicrosecond(1000000/4);
                        j++;
                        break;
                    case 3:
                        setMotor1(1023);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 1);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 0);
                        setMotor2(1023);
                        waitMicrosecond(1000000/4);
                        j = 0;
                        break;
                    }
        }
    }
    //negative number of steps means counterclockwise motion
    else if(steps < 0)
    {
        putsUart0("Counterclockwise\n");

        //turn the steps into a positive number for the loop
        for(i = 0; i < (steps*-1); i++)
        {
            switch(j)
                    {
                    case 0:
                        setMotor2(1023);
                        setPinValue(DIRA, 1);
                        setPinValue(DIRB, 0);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 0);
                        setMotor1(1023);
                        waitMicrosecond(1000000/4);
                        j = 3;
                        break;
                    case 1:
                        setMotor1(1023);
                        setPinValue(DIRC, 1);
                        setPinValue(DIRD, 0);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 0);
                        setMotor2(1023);
                        waitMicrosecond(1000000/4);
                        j--;
                        break;
                    case 2:
                        setMotor2(1023);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 1);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 0);
                        setMotor1(1023);
                        waitMicrosecond(1000000/4);
                        j--;
                        break;
                    case 3:
                        setMotor1(1023);
                        setPinValue(DIRC, 0);
                        setPinValue(DIRD, 1);
                        setPinValue(DIRA, 0);
                        setPinValue(DIRB, 0);
                        setMotor2(1023);
                        waitMicrosecond(1000000/4);
                        j--;
                        break;
                    }
        }
    }
    //if the user inputs 0
    else
    {
        putsUart0("No turning\n");
    }
}

void turnFourFullMicrosteps(int16_t steps)
{

    bool dir1 = 0;
    bool dir2 = 0;
    uint16_t k = 0;
    uint16_t j = 0;

    if (steps==0)
    {
        return;
    }

    //turn CCW
    else if (steps < 0)
    {
        for (k=0; k < (steps*-1); k++)
            {
                for (j=128; j > 0; j--)
                {
                    uint16_t motorspeed1 = (abs( 1023*sinarray[ (globalstep+j)%128 ] ) );
                    setMotor1 ( motorspeed1 );
                    if ( sinarray[ ((globalstep+j)%128) ] > 0)
                    {
                        dir1 = 1;
                    }

                    else
                    {
                        dir1 = 0;
                    }

                    setPinValue(DIRA,  dir1);
                    setPinValue(DIRB, !dir1);

                    uint16_t motorspeed2 = (abs( 1023*cosarray[ (globalstep+j)%128 ] ) );
                    setMotor2 ( motorspeed2 );
                    if ( cosarray[ ((globalstep+j)%128) ] > 0)
                    {
                        dir2 = 1;
                    }

                    else
                    {
                        dir2 = 0;
                    }

                    setPinValue(DIRC,  dir2);
                    setPinValue(DIRD, !dir2);

                    waitMicrosecond(200);
                }
            }
    }

    //turn CW
    else
    {
        for (k=0; k < steps; k++)
        {
            for (j=0; j < 128; j++)
            {
                uint16_t motorspeed1 = (abs( 1023*sinarray[ (globalstep+j)%128 ] ) );
                setMotor1 ( motorspeed1 );
                if ( sinarray[ ((globalstep+j)%128) ] > 0)
                {
                    dir1 = 1;
                }

                else
                {
                    dir1 = 0;
                }

                setPinValue(DIRA,  dir1);
                setPinValue(DIRB, !dir1);

                uint16_t motorspeed2 = (abs( 1023*cosarray[ (globalstep+j)%128 ] ) );
                setMotor2 ( motorspeed2 );
                if ( cosarray[ ((globalstep+j)%128) ] > 0)
                {
                    dir2 = 1;
                }

                else
                {
                    dir2 = 0;
                }

                setPinValue(DIRC,  dir2);
                setPinValue(DIRD, !dir2);
                waitMicrosecond(200);
            }
        }
    }
}

void turnRemainingMicrosteps(int16_t microsteps)
{
    bool dir1 = 0;
    bool dir2 = 0;
    uint16_t k = 0;

    if (microsteps==0)
    {
        return;
    }

    //turn CCW
    else if (microsteps < 0)
    {
        for (k=0; k < (microsteps*-1); k++)
        {
            setMotor1 ( (abs( 1023*sinarray[globalstep] ) ) );
            if ( sinarray[globalstep] > 0)
            {
                dir1 = 1;
            }

            else
            {
                dir1 = 0;
            }

            setPinValue(DIRA,  dir1);
            setPinValue(DIRB, !dir1);

            setMotor2 ( (abs( 1023*cosarray[globalstep] ) ) );
            if ( cosarray[globalstep] > 0)
            {
                dir2 = 1;
            }

            else
            {
                dir2 = 0;
            }

            setPinValue(DIRC,  dir2);
            setPinValue(DIRD, !dir2);

            globalstep--;
            if (globalstep < 0)
            {
                globalstep = 127;
            }

            waitMicrosecond(200);
        }
    }

    //turn CW
    else
    {
        for (k=0; k < microsteps; k++)
        {
            setMotor1 ( (abs( 1023*sinarray[globalstep] ) ) );
            if ( sinarray[globalstep] > 0)
            {
                dir1 = 1;
            }

            else
            {
                dir1 = 0;
            }

            setPinValue(DIRA,  dir1);
            setPinValue(DIRB, !dir1);

            setMotor2 ( (abs( 1023*cosarray[globalstep] ) ) );
            if ( cosarray[globalstep] > 0)
            {
                dir2 = 1;
            }

            else
            {
                dir2 = 0;
            }

            setPinValue(DIRC,  dir2);
            setPinValue(DIRD, !dir2);

            globalstep++;
            globalstep = (globalstep%128);

            waitMicrosecond(200);
        }
    }
}

void initHW()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; //enable port E
    enablePort(PORTE);

    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;  // enable motor
    _delay_cycles(3);
    enablePort(PORTF);

    // Configure three LEDs
    GPIO_PORTF_DEN_R |= MOTOR_MASK | RED_LED_MASK | GREEN_LED_MASK | BLUE_LED_MASK;
    GPIO_PORTF_AFSEL_R |= MOTOR_MASK | RED_LED_MASK | GREEN_LED_MASK | BLUE_LED_MASK;
    GPIO_PORTF_PCTL_R &= ~(GPIO_PCTL_PF0_M | GPIO_PCTL_PF1_M | GPIO_PCTL_PF2_M | GPIO_PCTL_PF3_M);
    GPIO_PORTF_PCTL_R |= GPIO_PCTL_PF0_M1PWM4 | GPIO_PCTL_PF1_M1PWM5 | GPIO_PCTL_PF2_M1PWM6 | GPIO_PCTL_PF3_M1PWM7;

    // Configure PWM module 1 to drive RGB LED
    // RED   on M1PWM5 (PF1), M1PWM2b
    // BLUE  on M1PWM6 (PF2), M1PWM3a
    // GREEN on M1PWM7 (PF3), M1PWM3b
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;                // reset PWM1 module
    SYSCTL_SRPWM_R = 0;                              // leave reset state
    PWM1_2_CTL_R = 0;                                // turn-off PWM1 generator 2 (drives outs 4 and 5)
    PWM1_3_CTL_R = 0;                                // turn-off PWM1 generator 3 (drives outs 6 and 7)
    PWM1_2_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
                                                             // output 5 on PWM1, gen 2b, cmpb
    PWM1_3_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;
                                                             // output 6 on PWM1, gen 3a, cmpa
    PWM1_3_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
                                                             // output 7 on PWM1, gen 3b, cmpb

    PWM1_2_LOAD_R = 1024;                            // set frequency to 40 MHz sys clock / 2 / 1024 = 19.53125 kHz
    PWM1_3_LOAD_R = 1024;                            // (internal counter counts down from load value to zero)

    PWM1_2_CMPB_R = 0;                               // red off (0=always low, 1023=always high)
    PWM1_3_CMPB_R = 0;                               // green off
    PWM1_3_CMPA_R = 0;                               // blue off

    PWM1_2_CTL_R = PWM_1_CTL_ENABLE;                 // turn-on PWM1 generator 2
    PWM1_3_CTL_R = PWM_1_CTL_ENABLE;                 // turn-on PWM1 generator 3
    PWM1_ENABLE_R = PWM_ENABLE_PWM5EN | PWM_ENABLE_PWM6EN | PWM_ENABLE_PWM7EN;

}

int main(void)
{
    initHW();
    initUart0();
    setUart0BaudRate(115200, 40e6);

    selectPinPushPullOutput(DIRA);
    selectPinPushPullOutput(DIRB);
    selectPinPushPullOutput(DIRC);
    selectPinPushPullOutput(DIRD);
    selectPinDigitalInput(OPTICALSENS);

    USER_DATA data;
    //Leveling the beam
    resetBar();

    while (1)
    {
    putsUart0("What angle do you want?\n");
    getsUart0(&data);

    //angle given by the user
    float angle = atof(data.buffer);

    float delta = angle - position;
    int angletemp = angle;
    position = angle;

    //angle converted to number of steps (stored in float)
    float fullsteps = (delta/7.2);

    //rounding the float value to a whole number
    int intfullsteps = fullsteps/1;

    //
    int remainingsteps = (delta-(intfullsteps)*7.2) / 0.05625;

    //takes the number of full steps and turns accordingly
    turnFourFullMicrosteps(intfullsteps);

    //takes the number of remaining microsteps and turns accordingly
    turnRemainingMicrosteps(remainingsteps);

    waitMicrosecond(100);
    }

    return 0;
}
