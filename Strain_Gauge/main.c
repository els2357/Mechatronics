//Luc Pham
//Ethan Sprinkle
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "gpio.h"
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

uint32_t data = 0b00000000000000000000000000000000;

initHw()
{
    initSystemClockTo40Mhz();
    enablePort(PORTA);
    selectPinPushPullOutput(PORTA, 2); //CLK
    selectPinDigitalInput(PORTA, 3);   //DATA
}

int main(void)
{
    initHw();
    initUart0();
    setUart0BaudRate(115200, 40e6);
    int8_t i = 0;
    char str[40];
    uint32_t drift = 0;
    //uint32_t weight = 0;
    uint32_t averageBits = 0;
    uint32_t previous = 0;
    uint32_t weight[40];
    int k = 0;
    int j = 0;
    uint32_t averageWeight = 0;
    uint32_t sum = 0;
    uint8_t check = 0;

    while(true)
    {
        data = 0;
        while(getPinValue(PORTA, 3) == 1);

        for(i = 23; i >= 0; i--)
        {
            setPinValue(PORTA, 2, 1);
            waitMicrosecond(40);
            data |= (uint32_t)(getPinValue(PORTA, 3) << i) ;
            setPinValue(PORTA, 2, 0);
            waitMicrosecond(1);
        }

        if(data < (previous - 1000))
        {
            putsUart0("Entered Loop\n\n\n");
            check = 1;
            weight[k] = data;
            if(k == 39)
            {
                k = 0;
                check = 0;
                for(j = 0; j < 40; j++)
                {
                    sum = sum + weight[j];
                }
                averageWeight = sum/40;
                averageBits = previous - averageWeight;

                sprintf(str, "     average Bit: %"PRIu32" \n", averageWeight);
                putsUart0(str);

                sprintf(str, "     Difference: %"PRIu32" \n", averageBits );
                putsUart0(str);

                sprintf(str, "     Grams: %f \n", (averageBits*0.0234 + 5.5876));
                putsUart0(str);
                sum = 0;
                averageWeight = 0;
                averageBits = 0;
            }
            k++;

            if(k == 40)
            {
                k = 0;
            }

        }

        if(check == 0)
        {
            previous = data;
        }



        setPinValue(PORTA, 2, 1);
        waitMicrosecond(40);
        setPinValue(PORTA, 2, 0);
        waitMicrosecond(1);

        //final = weight - drift;
        sprintf(str, "24bit number: %"PRIu32" \n", data);
        putsUart0(str);

    }

    /*
    while(true)
    {
        data = 0;
        while(getPinValue(PORTA, 3) != 1);

        for(i = 23; i > 0; i--)
        {
            setPinValue(PORTA, 2, 1);
            waitMicrosecond(1);
            data = ( data | (getPinValue(PORTA, 3) << i) );
            setPinValue(PORTA, 2, 0);
            waitMicrosecond(1);
            //_delay_cycles(3); //75 nanoseconds
        }
        setPinValue(PORTA, 2, 1);
        waitMicrosecond(1);
        data = ( data | getPinValue(PORTA, 3) );
        setPinValue(PORTA, 2, 0);
        waitMicrosecond(1);

        setPinValue(PORTA, 2, 1);
        waitMicrosecond(1);
        setPinValue(PORTA, 2, 0);

        sprintf(str, "24bit number: %"PRIu32" \n", data);
        putsUart0(str);

        //waitMicrosecond(500000);
    }
    */



	return 0;
}
