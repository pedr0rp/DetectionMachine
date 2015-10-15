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
		save(s_fps);
		
	} else {
		s_fps = "";
	}

	if (counter == (INT_MAX - 1000)) { counter = 0; }
	return s_fps;

}

void FPS::save(std::string value) {

	if (!fileCreated) {
		time_t now = time(0);
		struct tm  tstruct;
		char buf[80];
		tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
		std::stringstream ss;
		ss << buf;
		ss >> fileName;
		fileCreated = true;
	}
	
	std::ofstream o(DIRECTORY + fileName + FILE_EXTENSION, std::ios::app);
	std::string content = value + ";";	

	o << content << std::endl;
}