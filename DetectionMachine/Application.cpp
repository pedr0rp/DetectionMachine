#include "Application.h"

bool Application::init() {

	bool ready = true;

	SP = new Serial("COM3");

	printf("Arduino");
	if (SP->IsConnected()) {
		printf(" connected");
		//move_cam(90, 90);
	} else {
		printf(" disconnected");
		ready = false;
	}
	
	printf("\n\n");

	printf("Source: ");
	scanf("%d", &source);

	if (source) {
		cv::VideoCapture cap(CAM);

		printf("\tWeb cam");
		if (cap.isOpened()) {
			printf(" connected\n");
		} else {
			printf("\t disconnected\n");
			ready = false;
		}
		cap.release();
	} else {
		bool fileFound = false;
		do {
			printf("\tFilename: ");
			scanf("%s", &imageFile);
			std::ifstream f(imageFile);
			fileFound = f.good();
			f.close();

		} while (!fileFound);

	}
	return ready;
}

void Application::calibrate() {

	int temp;
	printf("\nHow many colors? ");
	scanf("%d", &temp);

	int load = 0;
	printf("Load? ");
	scanf("%d", &load);

	if (load) {
		for (int i = 0; i < temp; i++) {
			char name[10];
			printf("\tColor name: ");
			scanf("%s", &name);
			colors[i].setName(name);
			printf("\tLoading %s... ", name);
			if (colors[i].load()) {
				printf("OK!\n");
				colorCount++;
			} else {
				printf("ERROR!\n");
			}
		}
	} else {
		colorCount = temp;

		for (int i = 0; i < colorCount; i++) {
			char name[10];
			printf("Name color: ");
			scanf("%s", &name);
			colors[i].setName(name);
			printf("\tCalibrating color %s... ", name);
			colors[i].showControl();

			bool next = false;

			cv::Mat src;
			cv::Mat thresholded;			
			cv::VideoCapture cap(CAM);
			
			while (!next) {		

				src = readImage();
				thresholded = threshold(src, colors[i]);				

				imshow("Original", src);
				imshow("Thresholded", thresholded);

				switch (cv::waitKey(10)) {
				case 27:
					printf("OK!\n");
					cv::destroyAllWindows();
					next = true;
					break;
				}
			}
			cap.release();
		}
	}
}

cv::Mat Application::threshold(cv::Mat src, HSV color) {

	cv::Mat thresholded;

	cv::cvtColor(src, thresholded, cv::COLOR_BGR2HSV);
	cv::inRange(thresholded, color.getLow(), color.getHigh(), thresholded);

	int shape = cv::MORPH_ELLIPSE;
	cv::Size size = cv::Size(5, 5);

	cv::Mat element = cv::getStructuringElement(shape, size);


	erode(thresholded, thresholded, element);
	dilate(thresholded, thresholded, element);

	dilate(thresholded, thresholded, element);
	erode(thresholded, thresholded, element);

	return thresholded;

}

cv::Mat Application::preprocessing(cv::Mat src, HSV color) {

	src = threshold(src, color);	

	cv::GaussianBlur(src, src, cv::Size(3, 3), 2, 2);
	cv::Canny(src, src, 60, 180, 3);

	return src;
}

std::vector<Object> Application::findCircles(cv::Mat src, HSV color) {
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(src, circles, CV_HOUGH_GRADIENT, 2, src.rows / 8, 80, 80, 10, 0);
	
	std::vector<Object> objects; 

	for (size_t i = 0; i < circles.size(); i++) {				
		Object circle;
		circle.setPosition(cv::Point(cvRound(circles[i][0]), cvRound(circles[i][1])));
		circle.setShape(Shape::CIRCLE);
		circle.setColor(color.getName());
		objects.push_back(circle);
	}

	return objects;
}

static double angle(cv::Point p1, cv::Point p2, cv::Point p0)
{
	double dx1 = p1.x - p0.x;
	double dy1 = p1.y - p0.y;
	double dx2 = p2.x - p0.x;
	double dy2 = p2.y - p0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

std::vector<Object> Application::findPoly(cv::Mat src, HSV color) {

	std::vector<Object> objects;
	
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(src, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	
	std::vector<cv::Point> approx;
	
	for (int i = 0; i < contours.size(); i++) {

		approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);
		if (fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx)) continue;
		
		if (approx.size() == 3)	{ 
			Object object;
			object.setColor(color.getName());

			int x, y;
			x = y = 0;
			for (int i = 0; i < approx.size(); i++) {
				x += approx[i].x;
				y += approx[i].y;

			}
			object.setPosition(cv::Point(cvRound(x / approx.size()), cvRound(y / approx.size())));
			object.setShape(Shape::TRIANGLE);
			objects.push_back(object);
		}
		if (approx.size() == 4) {
											
			std::vector<double> cos;
			for (int j = 2; j < approx.size() + 1; j++) { 
				cos.push_back(angle(approx[j%approx.size()], approx[j-2], approx[j-1])); 
			}
			sort(cos.begin(), cos.end());
	
			if (cos.front() >= -0.3 && cos.back() <= 0.3) {
				Object object;
				object.setColor(color.getName());

				int x, y;
				x = y = 0;
				for (int i = 0; i < approx.size(); i++) {
					x += approx[i].x;
					y += approx[i].y;

				}
				object.setPosition(cv::Point(cvRound(x / approx.size()), cvRound(y / approx.size())));
				object.setShape(Shape::RECTANGLE);
				objects.push_back(object);
			}						
		}	

		
	}

	return objects;
}


void Application::drawObject(cv::Mat &src, Object object) {
	
	cv::circle(src, object.getPosition(), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
	
}

cv::Mat Application::readImage() {
	cv::Mat src;

	if (source) { 
		cv::VideoCapture cap(CAM);
		if (!cap.read(src)) { 
			printf("Cannot read a frame from video stream\n"); 
		}
		cap.release();
	}
	else { 
		src = cv::imread(imageFile); 
	}

	return src;
}

int Application::start() {
	init();
	calibrate();

	cv::Mat src;
	cv::Mat processed[MAX_COLOR];

	int a = 80;
	int b = 80;

	while (true) {
		framerate.start();
		src = readImage();

		std::vector<std::future<cv::Mat>> t_preprocessing(MAX_COLOR);

		for (int i = 0; i < colorCount; i++) { 
			HSV color = colors[i];
			t_preprocessing[i] = std::async(std::launch::async, [src, color] { return preprocessing(src, color); });
		}
		
		for (int i = 0; i < colorCount; i++) { t_preprocessing[i].wait(); }
		for (int i = 0; i < colorCount; i++) { processed[i] = t_preprocessing[i].get(); }

		std::vector<std::future<std::vector<Object>>> t_circles(MAX_COLOR);
		for (int i = 0; i < colorCount; i++) {			
			cv::Mat temp = processed[i];
			HSV color = colors[i];
			t_circles[i] = std::async(std::launch::async, [temp, color, a, b] { return findCircles(temp, color); });
		}

		for (int i = 0; i < colorCount; i++) {
			t_circles[i].wait();			
		}

		std::vector<std::future<std::vector<Object>>> t_poly(MAX_COLOR);
		for (int i = 0; i < colorCount; i++) {
			cv::Mat temp = processed[i];
			HSV color = colors[i];
			t_poly[i] = std::async(std::launch::async, [temp, color] { return findPoly(temp, color); });
		}

		for (int i = 0; i < colorCount; i++) { 
			t_poly[i].wait(); 
		}
		
		for (int i = 0; i < colorCount; i++) { 				
			std::vector<Object> circles = t_circles[i].get();			
			objects.insert(objects.end(), circles.begin(), circles.end());

			std::vector<Object> polys = t_poly[i].get();
			objects.insert(objects.end(), polys.begin(), polys.end());
		}

		for (int i = 0; i < objects.size(); i++) {
			drawObject(src, objects[i]);
		}


		putText(src, framerate.end(), cv::Point(src.size().width - 70, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(0, 0, 0), 1, CV_AA);		
		imshow("Source", src);
		objects.clear();

		switch (cv::waitKey(10)) {			
			case 27:
				cv::destroyAllWindows();
				for (int i = 0; i < colorCount; i++) { colors[i].save(); }
			return 0;
		}
	}

	return 0;
}