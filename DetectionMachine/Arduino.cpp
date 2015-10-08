#include <stdio.h>
#include <tchar.h>
#include "Serial.h"	
#include <string>

// application reads from the specified serial port and reports the collected data
int _tmain(int argc, _TCHAR* argv[])
{

	Serial* SP = new Serial("COM3");    // adjust as needed

	if (SP->IsConnected())
		printf("Connected\n");

	char incomingData[256] = "";			// don't forget to pre-allocate memory
	//printf("%s\n",incomingData);
	int dataLength = 3;
	int readResult = 0;

	int value = 0;

	//while (false)
	while (SP->IsConnected())
	{
		char numberstring[(((sizeof value) * CHAR_BIT) + 2) / 3 + 2];
		sprintf(numberstring, "%03d", value);
		
		printf("Sending %s ->  %d \n", numberstring, SP->WriteData(numberstring, dataLength));

		
		if (value <= 170) {
			value+=10;
		}
		else {
			value = 0;
		}

		Sleep(200);
		system("Pause");

		
	}
	system("Pause");
	return 0;
}
