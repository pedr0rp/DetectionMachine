#include "FPS.h"

void FPS::start() {
	if (counter == 0) { 
		time(&_start);
	}
}

std::string FPS::end() {

	time(&_end);
	counter++;
	sec = difftime(_end, _start);
	fps = counter/sec;

	std::string s_fps;

	if (counter > 30) {
		std::ostringstream out;
		out << std::setprecision(3) << fps;
		s_fps = out.str();

		return s_fps;
	}
	else {
		return "";
	}
	if (counter == (INT_MAX - 1000)) { counter = 0; }

}