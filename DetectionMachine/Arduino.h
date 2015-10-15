#ifndef ARDUINO_H
#define ARDUINO_H

#include <string>
#include "Serial.h"

#define USB_PORT   "COM3"
#define MAX_X	    179
#define MIN_X	    0
#define MAX_Y	    315
#define MIN_Y	    180
#define DATA_LENGHT 3

class Arduino {

private:
	static int int_;
	int x, y;
	Serial* SP;
	int readResult = 0;
	char numberstring[(((sizeof int_) * CHAR_BIT) + 2) / 3 + 2];
	int step = 2;

	void send(int x, int y);


public:
	Arduino();
	bool isConnected();

	void up();
	void down();
	void left();
	void right();
};

#endif 
