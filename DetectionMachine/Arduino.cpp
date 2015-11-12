#include "Arduino.h"


Arduino::Arduino() {
	SP = new Serial("COM3");
	send(90, 180);
}

bool Arduino::isConnected() {
	return SP->IsConnected();
}

void Arduino::send(int x, int y) {

	if (x >= MIN_X && x <= MAX_X) { 
		Arduino::x = x;
		sprintf(numberstring, "%03d", Arduino::x);
		SP->WriteData(numberstring, DATA_LENGHT);
	}
	if (y + 180 >= MIN_Y && y + 180 <= MAX_Y) {
		Arduino::y = y;
		sprintf(numberstring, "%03d", Arduino::y + 181);
		SP->WriteData(numberstring, DATA_LENGHT);
	}
}

void Arduino::up() {
	send(x, y - step);	
}

void Arduino::down() {
	send(x, y + step);
}

void Arduino::left() {
	send(x - step, y);
}

void Arduino::right() {
	send(x + step, y);		
}