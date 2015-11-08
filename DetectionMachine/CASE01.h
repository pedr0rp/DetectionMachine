#ifndef CASE01_H
#define CASE01_H

#include "Util.h";

#define MAX_COLOR	10
#define MAX_OBJECT  20
#define CAM			1	

class CASE01 {

private:
	bool TRACKING = false;
	bool SHOW_PROCESSED = true;
	bool INCLUDE_TEXT = true;
	bool INCLUDE_AIM = false;
	bool INCLUDE_DISTANCE = true;
	
	FPS framerate;
	Arduino arduino;

	time_t _start;
	int time;

	int source;
	char imageFile[20];

	HSV colors[MAX_COLOR];
	int colorCount;

	std::vector<Object*> objects;

	int index;

public:
	bool init();
	int start();
	void calibrate();
	static void mouseCapture(int event, int x, int y, int flags, void* userdata);
};


#endif 