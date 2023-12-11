//Luc Pham
//Ethan Sprinkle

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "gpio.h"
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "uart1.h"
#include "tm4c123gh6pm.h"

struct polarCoordinate{
    uint16_t angle;
    uint16_t distance;
};

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
    uint16_t scan[5];
    uint32_t i = 0;
	uint32_t j = 0;
	uint8_t flag = 0;
    char str[40];
	struct polarCoordinate polarCoordinates[500];
	struct polarCoordinate temp;

	//Pulse Motor
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

	//Read Response Descriptor bytes
    for(i = 0; i < 7; i++)
    {
        responseDescriptor[i] = getcUart1();
    }

	//Read Info Data
    for(i = 0; i <20; i++)
    {
        data[i] = getcUart1();
    }
	
	//Outout Info Data to UART0
    /*
	sprintf(str, "Model: %X \n", data[0] );
    putsUart0(str);
	
	sprintf(str, "Firmware Version: %d.", data[2] );
    putsUart0(str);
	
	sprintf(str, "%d \n", data[1] );
    putsUart0(str);
	
	sprintf(str, "Hardware Version", data[3] );
    putsUart0(str);
	
	sprintf(str, "Serial Number: ");
    putsUart0(str);

	
	 for(i = 4; i < 20; i++)
    {
        sprintf(str, "%X", i, data[i] );
        putsUart0(str);
    }
	
	sprintf(str, "\n");
    putsUart0(str);
	*/

    waitMicrosecond(1001);

	//Turn on Motor and Wait 2 Seconds to get up to speed
    setPinValue(PORTB, 2, 1);
    waitMicrosecond(2000000);

    //SCAN COMMAND
    putcUart1(0xA5);
    putcUart1(0x20);

	//Read Response Descriptor bytes
    for(i = 0; i < 7; i++)
    {
        responseDescriptor[i] = getcUart1();
    }

    while(j < 500)
    {
		for(i = 0; i < 5; i++)
		{
			scan[i] = getcUart1();
		}
		//scan[0] - may use new scan bit, but otherwise ignore
		//polarCoordinates[j].start = scan[0] & 1;
		//scan[1] - copy bits 7:1 to angle 6:0
		//scan[2] - copy bits 7:0 to angle 14:7
		polarCoordinates[j].angle = (scan[1]>> 1) |  (scan[2]<< 7);
		//int k = polarCoordinates[j].angle;
		
		//scan[3] - copy bits 7:0 to distance 7:0
		//scan[4] - copy bits 7:0 to distance 14:7
		uint32_t tempdistance = (scan[3]) |  (scan[4]<< 8);
		if (tempdistance != 0)
		{
		    polarCoordinates[j].distance = (scan[3]) |  (scan[4]<< 8);
		    j++;
		}
    }
	
	//Send STOP
    putcUart1(0xA5);
    putcUart1(0x25);
    waitMicrosecond(1001);
	
	//Stop Motor
	setPinValue(PORTB, 2, 0);

	putsUart0("\n");
	//Calculate Room Area using Coordinates
	// Law of Sines
	// Area = 0.5 * d1 * d2 * sin(theta)

	//Sort array
	for(i = 0; i < 500; i++)
	{
	    for (j = 0; j < 500 - i; j++)
	    {
	        if ( polarCoordinates[j].angle/64 > polarCoordinates[j+1].angle/64 )
            {
	            temp = polarCoordinates[j+1];
	            polarCoordinates[j+1] = polarCoordinates[j];
	            polarCoordinates[j] = temp;
            }
	    }
	}
	double area = 0;
	float temparea[499];
	float angledifference = 0;

    for (i = 0; i < 499; i++)
    {
        //sprintf(str, "Start Bit: %d \n", polarCoordinates[i].start );
        //putsUart0(str);
        sprintf(str, "%d, %d \n", ((polarCoordinates[i].angle)/64), ((polarCoordinates[i].distance)/4));
        putsUart0(str);
        angledifference = (polarCoordinates[i+1].angle/64) - (polarCoordinates[i].angle/64);
        temparea[i] = 0.5 * (polarCoordinates[i].distance/4) * (polarCoordinates[i+1].distance/4) * sin(angledifference*3.14/180 );
        area = temparea[i] + area;
        //sprintf(str, "Distance: %d mm\n", (polarCoordinates[i].distance)/4 );
        //putsUart0(str);
    }
    sprintf(str, "Area: %f square inches\n", area * 0.00155 );
    putsUart0(str);

	while(true);
	return 0;
}
