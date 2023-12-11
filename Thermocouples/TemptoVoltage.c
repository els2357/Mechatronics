#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "gpio.h"
#include "wait.h"
#include "uart0.h"
#include "clock.h"
#include "tm4c123gh6pm.h"
#include "i2c0.h"


// 0        1       2       3       4       5       6       7       8       9
float milivoltages[381] = 
{-2.920, -2.887, -2.854, -2.821, -2.788, -2.755, -2.721, -2.688, -2.654, -2.620, //-80
 -2.587, -2.553, -2.519, -2.485, -2.450, -2.416, -2.382, -2.347, -2.312, -2.278, //-70
 -2.243, -2.208, -2.173, -2.138, -2.103, -2.067, -2.032, -1.996, -1.961, -1.925, //-60
 -1.889, -1.854, -1.818, -1.782, -1.745, -1.709, -1.673, -1.637, -1.600, -1.564, //-50
 -1.527, -1.490, -1.453, -1.417, -1.380, -1.343, -1.305, -1.268, -1.231, -1.194, //-40
 -1.156, -1.119, -1.081, -1.043, -1.006, -0.968, -0.930, -0.892, -0.854, -0.816, //-30
 -0.778, -0.739, -0.701, -0.663, -0.624, -0.586, -0.547, -0.508, -0.470, -0.431, //-20
 -0.392, -0.353, -0.314, -0.275, -0.236, -0.197, -0.157, -0.118, -0.079, -0.039, //-10
  0.000,  0.039,  0.079,  0.119,  0.158,  0.198,  0.238,  0.277,  0.317,  0.357, //  0
  0.397,  0.437,  0.477,  0.517,  0.557,  0.597,  0.637,  0.677,  0.718,  0.758, // 10
  0.798,  0.838,  0.879,  0.919,  0.960,  1.000,  1.041,  1.081,  1.122,  1.163, // 20
  1.203,  1.244,  1.285,  1.326,  1.366,  1.407,  1.448,  1.489,  1.530,  1.571, // 30
  1.612,  1.653,  1.694,  1.735,  1.776,  1.817,  1.858,  1.899,  1.941,  1.982, // 40
  2.023,  2.064,  2.106,  2.147,  2.188,  2.230,  2.271,  2.312,  2.354,  2.395, // 50
  2.436,  2.478,  2.519,  2.561,  2.602,  2.644,  2.685,  2.727,  2.768,  2.810, // 60
  2.851,  2.893,  2.934,  2.976,  3.017,  3.059,  3.100,  3.142,  3.184,  3.225, // 70
  3.267,  3.308,  3.350,  3.391,  3.433,  3.474,  3.516,  3.557,  3.599,  3.640, // 80
  3.682,  3.723,  3.765,  3.806,  3.848,  3.889,  3.931,  3.972,  4.013,  4.055, // 90
  4.096,  4.138,  4.179,  4.220,  4.262,  4.303,  4.344,  4.385,  4.427,  4.468, //100
  4.509,  4.550,  4.591,  4.633,  4.674,  4.715,  4.756,  4.797,  4.838,  4.879, //110
  4.920,  4.961,  5.002,  5.043,  5.084,  5.124,  5.165,  5.206,  5.247,  5.288, //120
  5.328,  5.369,  5.410,  5.450,  5.491,  5.532,  5.572,  5.613,  5.653,  5.694, //130
  5.735,  5.775,  5.815,  5.856,  5.896,  5.937,  5.977,  6.017,  6.058,  6.098, //140
  6.138,  6.179,  6.219,  6.259,  6.299,  6.339,  6.380,  6.420,  6.460,  6.500, //150
  6.540,  6.580,  6.620,  6.660,  6.701,  6.741,  6.781,  6.821,  6.861,  6.901, //160
  6.941,  6.981,  7.021,  7.060,  7.100,  7.140,  7.180,  7.220,  7.260,  7.300, //170
  7.340,  7.380,  7.420,  7.460,  7.500,  7.540,  7.579,  7.619,  7.659,  7.699, //180
  7.739,  7.779,  7.819,  7.859,  7.899,  7.939,  7.979,  8.019,  8.059,  8.099, //190
  8.138,  8.178,  8.218,  8.258,  8.298,  8.338,  8.378,  8.418,  8.458,  8.499, //200
  8.539,  8.579,  8.619,  8.659,  8.699,  8.739,  8.779,  8.819,  8.860,  8.900, //210
  8.940,  8.980,  9.020,  9.061,  9.101,  9.141,  9.181,  9.222,  9.262,  9.302, //220
  9.343,  9.383,  9.423,  9.464,  9.504,  9.545,  9.585,  9.626,  9.666,  9.707, //230
  9.747,  9.788,  9.828,  9.869,  9.909,  9.950,  9.991, 10.031, 10.072, 10.113, //240
 10.153, 10.194, 10.235, 10.276, 10.316, 10.357, 10.398, 10.439, 10.480, 10.520, //250
 10.561, 10.602, 10.643, 10.684, 10.725, 10.766, 10.807, 10.848, 10.889, 10.930, //260
 10.971, 11.012, 11.053, 11.094, 11.135, 11.176, 11.217, 11.259, 11.300, 11.341, //270
 11.382, 11.423, 11.465, 11.506, 11.547, 11.588, 11.630, 11.671, 11.712, 11.753, //280
 11.795, 11.836, 11.877, 11.919, 11.960, 12.001, 12.043, 12.084, 12.126, 12.167, //290
 12.209};

float voltageToTemp(float voltage)
{
	//First, find the two relevant values in the array
	//while loop breaks when the next highest voltage is found
	int i = 0;
	while(milivoltages[i] < voltage)
	{
		i++;
		
		//if voltage is greater than array, return max temp of 300
		if (i > 380)
		{
			return 300;
		}
	}
	
	//break and return minimum temp value of -80 if voltage is less than -2.920
	if(i == 0)
	{
		return -80;
	}
	
	//y=temp, x=voltgae
	float x = voltage;
	float y1 = i - 80;
	float x1 = milivoltages[i];
	float y2 = i - 81;
	float x2 = milivoltages[i-1];
	
	//linear interprolation formula y = y1 + ((x-x1)(y2-y1)) / (x2-x1)
	float y = (y1 + (((x-x1)*(y2-y1)) / (x2-x1)));
	
	return y;
}

float tempToVoltage(float temp)
{
	//First, find the two relevant indexes in the array by taking temp and subtracting 80
	int index1 = (temp + 80)/1; //takes the floor 
	int index2 = index1 + 1;
	
	
	if (index1 < 0)
	{
		return -2.920;
	}
	
	if (index2 > 380)
	{
		return 12.209;
	}
	
	//y=voltage, x=index
	float x = temp + 80;
	float x1 = index1;
	float y1 = milivoltages[index1];
	float x2 = index2;
	float y2 = milivoltages[index2];
	
	//linear interprolation formula y = y1 + ((x-x1)(y2-y1)) / (x2-x1)
	float y  = y1 + (((x-x1)*(y2-y1)) / (x2-x1));
	
	return y;
}//300

void initHw()
{
    initSystemClockTo40Mhz();
}

int main(void)
{
    initHw();
    initUart0();
    initI2c0();
    setUart0BaudRate(115200, 40e6);

    while(true)
    {
    //////////////////////////////////////////TMP36 VOLTAGE//////////////////////////////////////////////
    uint8_t output[2];
    uint8_t data[2] = { 0xE4, 0x03 };

    writeI2c0Registers(0x48, 0x01, data, 2);
    waitMicrosecond(1000000);
    readI2c0Registers(0x48, 0x00, output, 2);

    uint16_t MSB = output[0]<<8;
    uint16_t LSB = output[1] | 0x00;
    uint16_t FINAL = MSB | LSB;

    uint16_t testvalue = 11488; //used to test, replace "testvalue" with "FINAL" when we actually use the real circuit

    float voltage1 = (FINAL * 0.0000625);
    float tempCJ = 100*(voltage1 - 0.5);
    float voltageCJ = tempToVoltage(tempCJ);
    char str[40];

    /////////////////////////THERMOCOUPLER DIFFERENTIAL VOLTAGE///////////////////////////////////////////
    uint8_t output2[2];
    uint8_t data2[2] = { 0x8E, 0x00 };

    //How to find the thermocoupler differential votage
    writeI2c0Registers(0x48, 0x01, data2, 2);
    waitMicrosecond(1000000);
    readI2c0Registers(0x48, 0x00, output2, 2);

    uint16_t MSB2 = output2[0]<<8;
    uint16_t LSB2 = output2[1] | 0x00;
    int16_t FINAL2 = MSB2 | LSB2;

    //to account for different units, FINAL2 is multipled by 0.0078125 instead of 0.0000078125
    float voltage2 = (FINAL2 * 0.0078125);
    float sum = voltageCJ + voltage2;
    float TempTC = voltageToTemp(sum);
    float voltageTC = tempToVoltage(TempTC);


    ////////////////////OUTPUT TO UART///////////////////////////////////////////////////////////////////

    sprintf(str, "Value of voltage from TMP: %f V\n", (FINAL * 0.0000625));
    putsUart0(str);
    sprintf(str, "Value of temperatureCJ: %f degrees celsius\n", tempCJ);
    putsUart0(str);
    sprintf(str, "Value of voltageCJ: %f V\n\n", voltageCJ);
    putsUart0(str);

    sprintf(str, "Value of voltage of thermocoupler: %f V\n", (FINAL2 * 0.0000078125));
    putsUart0(str);
    sprintf(str, "Value of temperatureTC: %f degrees celsius\n", TempTC);
    putsUart0(str);
    sprintf(str, "Value of voltageTC: %f mV\n\n", voltageTC);
    putsUart0(str);

    if(TempTC >= 298)
    {
        putsUart0("DISCONNECTED");
    }

    }

    return 0;
}
