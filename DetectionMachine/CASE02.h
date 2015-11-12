#ifndef CASE02_H
#define CASE02_H

#include "Util.h";

#define MAX_COLOR	10
#define MAX_OBJECT  20
#define CAM			1	

class CASE02 {


private:
	bool SHOW_PROCESSED = false;
	bool INCLUDE_TEXT = false;

	FPS framerate;

	time_t _start;
	int time;

	int source;
	char imageFile[20];

	int keyColor;
	HSV colors[MAX_COLOR];
	int colorCount;

	std::vector<Object*> objects;
	std::vector<Object*> spaces;

	int index;

public:
	bool init();
	int start();
	void calibrate();
	static void mouseCapture(int event, int x, int y, int flags, void* userdata);
};


#endif 