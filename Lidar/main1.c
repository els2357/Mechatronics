//Luc Pham
//Ethan Sprinkle
/*
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "gpio.h"
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "uart1.h"
#include "tm4c123gh6pm.h"

void initHw()
{
    initSystemClockTo40Mhz();
    enablePort(PORTB);
    selectPinPushPullOutput(PORTB, 2);
}
int main(void)
{
    initHw();
    initUart0();
    setUart0BaudRate(115200, 40e6);
    initUart1();
    setUart1BaudRate(115200, 40e6);

    uint8_t responseDescriptor[7];
    uint8_t data[20];
    uint8_t scan[5];
    uint8_t i = 0;
    char str[40];

    setPinValue(PORTB, 2, 1);
    waitMicrosecond(1000000);
    setPinValue(PORTB, 2, 0);

    //Send STOP
    putcUart1(0xA5);
    putcUart1(0x25);
    waitMicrosecond(1001);

    //Send INFO
    putcUart1(0xA5);
    putcUart1(0x50);

    for(i = 0; i < 7; i++)
    {
        responseDescriptor[i] = getcUart1();
    }

    for(i = 0; i <20; i++)
    {
        data[i] = getcUart1();
    }

    for(i = 0; i < 20; i++)
    {
        sprintf(str, "data at %d: %X \n", i, data[i] );
        putsUart0(str);
    }
    waitMicrosecond(1001);

    setPinValue(PORTB, 2, 1);
    waitMicrosecond(2000000);

    //SCAN COMMAND
    putcUart1(0xA5);
    putcUart1(0x20);

    for(i = 0; i < 7; i++)
    {
        responseDescriptor[i] = getcUart1();
    }
    for(i = 0; i < 5; i++)
    {
        scan[i] = getcUart1();
    }

    while(true)
    {

    }
	return 0;
}
*/
