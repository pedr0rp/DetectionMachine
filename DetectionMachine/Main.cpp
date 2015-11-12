#include "CASE01.h"
#include "CASE02.h"


int main(int argc, char** argv) {

	cv::useOptimized();
	CASE02 app;
	app.start();

	return 0;
}