// Luc Pham 1001918323
// Ethan Sprinkle 1002002357

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "gpio.h"
#include "wait.h"
#include "uart0.h"
#include "clock.h"
#include "tm4c123gh6pm.h"

#define A_DIR PORTA,2
#define A_EN PORTA,3
#define B_DIR PORTB,0
#define B_EN PORTB,5
#define C_DIR PORTC,4
#define C_EN PORTC,5

#define HallBrown PORTA,5
#define HallBlue PORTB,4
#define HallOrange PORTC,6

void initHw()
{
    initSystemClockTo40Mhz();
    enablePort(PORTA);
    enablePort(PORTB);
    enablePort(PORTC);

    selectPinPushPullOutput(A_DIR);
    selectPinPushPullOutput(A_EN);

    selectPinPushPullOutput(B_DIR);
    selectPinPushPullOutput(B_EN);

    selectPinPushPullOutput(C_DIR);
    selectPinPushPullOutput(C_EN);

    selectPinDigitalInput(HallBrown);
    selectPinDigitalInput(HallBlue);
    selectPinDigitalInput(HallOrange);
}

void setValues(uint8_t ADIR, bool AEN, uint8_t BDIR, bool BEN, uint8_t CDIR, bool CEN)
{
    setPinValue(A_EN, AEN);
    setPinValue(B_EN, BEN);
    setPinValue(C_EN, CEN);
    setPinValue(A_DIR, ADIR);
    setPinValue(B_DIR, BDIR);
    setPinValue(C_DIR, CDIR);

}

void loop()
{
    //don't care variable
    uint8_t x = 0;
    uint32_t time = 8000; //1285
    char line[40];

    while(true)
    {
        setValues(1 , 1, 0, 1, x, 0);
        waitMicrosecond(time);
        setValues(x , 0, 0, 1, 1, 1);
        waitMicrosecond(time);
        setValues(0 , 1, x, 0, 1, 1);
        waitMicrosecond(time);
        setValues(0 , 1, 1, 1, x, 0);
        waitMicrosecond(time);
        setValues(x , 0, 1, 1, 0, 1);
        waitMicrosecond(time);
        setValues(1 , 1, x, 0, 0, 1);
        waitMicrosecond(time);

        if(time >= 3225)
        {
            time = time - 10;
        }
        sprintf(line, "value of time: %" PRIu32 "\n", time);
        putsUart0(line);
    }
}

void loopWithHallEffects()
{
    uint8_t x = 0;
    uint32_t time = 8000; //1285
    char line[40];

    while(time != 3220)
    {
        setValues(1 , 1, 0, 1, x, 0);
        waitMicrosecond(time);
        setValues(x , 0, 0, 1, 1, 1);
        waitMicrosecond(time);
        setValues(0 , 1, x, 0, 1, 1);
        waitMicrosecond(time);
        setValues(0 , 1, 1, 1, x, 0);
        waitMicrosecond(time);
        setValues(x , 0, 1, 1, 0, 1);
        waitMicrosecond(time);
        setValues(1 , 1, x, 0, 0, 1);
        waitMicrosecond(time);

        if(time >= 3225)
        {
            time = time - 10;
        }
        sprintf(line, "value of time: %" PRIu32 "\n", time);
        putsUart0(line);
    }

    //setValues(1 , 1, 0, 1, x, 0);

    bool a;
    bool b;
    bool c;
    uint32_t var = 1;

    uint8_t byte = 0b00000000;

    while(true)
    {
        a = getPinValue(HallBrown);
        b = getPinValue(HallBlue);
        c = getPinValue(HallOrange);

        byte = (a<<2) | (b<<1) | c;

        switch(byte)
        {
        case 5:
            setValues(x , 0, 0, 1, 1, 1);
            break;
        case 1:
            setValues(0 , 1, x, 0, 1, 1);
            break;
        case 3:
            setValues(0 , 1, 1, 1, x, 0);
            break;
        case 2:
            setValues(x , 0, 1, 1, 0, 1);
            break;
        case 6:
            setValues(1 , 1, x, 0, 0, 1);
            break;
        case 4:
            setValues(1 , 1, 0, 1, x, 0);
            break;
        default:
            break;
        }

        /*
        if( a == 0 && b == 0 && c == 1 )
        {
            //waitMicrosecond(var);
            setValues(x , 0, 0, 1, 1, 1);
        }
        else if( a == 0 && b == 1 && c == 1 )
        {
            //waitMicrosecond(var);
            setValues(0 , 1, x, 0, 1, 1);
        }
        else if( a == 0 && b == 1 && c == 0 )
        {
            //waitMicrosecond(var);
            setValues(0 , 1, 1, 1, x, 0);
        }
        else if( a == 1 && b == 1 && c == 0 )
        {
            //waitMicrosecond(var);
            setValues(x , 0, 1, 1, 0, 1);
        }
        else if( a == 1 && b == 0 && c == 0 )
        {
            //waitMicrosecond(var);
            setValues(1 , 1, x, 0, 0, 1);
        }
        else if( a == 1 && b == 0 && c == 1 )
        {
            //waitMicrosecond(var);
            setValues(1 , 1, 0, 1, x, 0);
        }
        else
        {

        }


        if(time != 1)
        {
            time--;
        }
        waitMicrosecond(time);
        */
    }
}

int main(void)
{
    initHw();
    initUart0();
    setUart0BaudRate(115200, 40e6);

    uint8_t x = 0; //don't care variable
    //setValues(1 , 1, 0, 1, x, 0);

    //loop();
    loopWithHallEffects();


    while(true);
	return 0;
}
