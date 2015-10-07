#include "opencv2/imgproc/imgproc.hpp",
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <time.h> 
#include <limits.h> 
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <conio.h>
#include <thread>
#include "SerialClass.h"	
#include "Object.cpp"
#include "ControlHSV.cpp"
#include <future>
#include <iomanip>

using namespace cv;
using namespace std;

bool TRACKING = false;
bool PROCESSED = true;
bool INCLUDE_TEXT = true;
bool INCLUDE_AIM = false;
bool INCLUDE_DISTANCE = true;

Serial* SP;
int camX = 90;
int camY = 90 ;
char numberstring[(((sizeof camX) * CHAR_BIT) + 2) / 3 + 2];

VideoCapture cap(1);
Mat outerLayer;
Mat src;

ControlHSV colors[10];
int countColors = 0;

int source = 0;
char imageFile[20];

Object objects[20] = { Object(src), Object(src), Object(src), Object(src), Object(src), Object(src), Object(src), Object(src), Object(src), Object(src),
					   Object(src), Object(src), Object(src), Object(src), Object(src), Object(src), Object(src), Object(src), Object(src), Object(src)
					 };
int countObjects = 0;


int minId = 0;

void move_cam(int x, int y) {

	int MIN_CAMY = 180;
	int MAX_CAMY = 315;
	int MIN_CAMX = 0;
	int MAX_CAMX = 179;

	int dataLength = 3;

	if (x >= MIN_CAMX && x <= MAX_CAMX) { camX = x; }
	if (y + 180 >= MIN_CAMY && y + 180 <= MAX_CAMY) { camY = y; }

	if (camX == x) {
		sprintf(numberstring, "%03d", camX);
		SP->WriteData(numberstring, dataLength);
	}
	if (camY == y) {
		sprintf(numberstring, "%03d", 181 + camY);
		SP->WriteData(numberstring, dataLength);
	}

}

bool initialize() {

	bool ready = true;

	SP = new Serial("COM3");
	if (SP->IsConnected()) {
		printf("Arduino connected\n");
		move_cam(90, 90);
	}
	else {
		printf("Arduino disconnected\n");
		ready = false;
	}

	printf("Source: ");
	scanf("%d", &source);

	if (source) {
		if (cap.isOpened()) {
			printf("\tWeb cam connected\n");
		}
		else {
			printf("\tWeb cam disconnected\n");
			ready = false;
		}
	}
	else {		
	
		bool fileFound = false;
		do {
			printf("\tFile name: ");
			scanf("%s", &imageFile);
			ifstream f(imageFile);
			fileFound = f.good();			
			f.close();
		
		} while (!fileFound);
		
		src = imread(imageFile);

	}

	
	
	return ready;
}

void calibrate() {

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
				countColors++;
				
			}
			else {
				printf("ERROR!\n");
			}
		}
	}
	else {
		countColors = temp;

		for (int i = 0; i < countColors; i++) {
			char name[10];
			printf("Name color: ");
			scanf("%s", &name);
			colors[i].setName(name);
			printf("\tCalibrating color %s... ", name);
			colors[i].show();

			bool next = false;
			Mat thresholded;
			while (!next) {

				if (source) { cap.read(src); }

				cvtColor(src, thresholded, COLOR_BGR2HSV);
				inRange(thresholded, colors[i].getLow(), colors[i].getHigh(), thresholded);

				int shape = MORPH_ELLIPSE;
				Size size = Size(5, 5);

				erode(thresholded, thresholded, getStructuringElement(shape, size));
				dilate(thresholded, thresholded, getStructuringElement(shape, size));

				dilate(thresholded, thresholded, getStructuringElement(shape, size));
				erode(thresholded, thresholded, getStructuringElement(shape, size));

				imshow("Original", src);
				imshow("Thresholded", thresholded);

				switch (waitKey(10)) {
				case 27:
					printf("OK!\n");
					destroyAllWindows();
					next = true;
					break;
				}
			}
		}
	}
}

Mat hsvthreshold (Mat original, ControlHSV control) {

	Mat temp;

	cvtColor(original, temp, COLOR_BGR2HSV);
	inRange(temp, control.getLow(), control.getHigh(), temp);

	int shape = MORPH_ELLIPSE;
	Size size = Size(5, 5);

	erode(temp, temp, getStructuringElement(shape, size));
	dilate(temp, temp, getStructuringElement(shape, size));

	dilate(temp, temp, getStructuringElement(shape, size));
	erode(temp, temp, getStructuringElement(shape, size));

	return temp;
}

vector<Vec3f> findCircles(Mat src) {

	vector<Vec3f> circles;
	//GaussianBlur(src, src, Size(9, 9), 2, 2);
	//Canny(src, src, 5, 70, 3);
	HoughCircles(src, circles, CV_HOUGH_GRADIENT, 2, src.rows / 8, 80, 80, 10, 0);
	//HoughCircles(src, circles, CV_HOUGH_GRADIENT, 2, src.rows / 8, 80, 100, 10, 0);

	return circles;
}

static double angle(Point p1, Point p2, Point p0)
{
	double dx1 = p1.x - p0.x;
	double dy1 = p1.y - p0.y;
	double dx2 = p2.x - p0.x;
	double dy2 = p2.y - p0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

void addObject(Point center, Shape shape, String color) {

	objects[countObjects].setPosition(center);
	objects[countObjects].setShape(shape);
	objects[countObjects].setColor(color);
	countObjects++;
}

void drawCircle(Mat src, Vec3f v) {

	Point center(cvRound(v[0]), cvRound(v[1]));
	int radius = cvRound(v[2]);
	circle(src, center, 3, Scalar(0, 255, 0), -1, 8);
	circle(src, center, radius, Scalar(0, 0, 255), 3, 8);
}

Point drawPolygon(Mat src, vector<Point> v) {

	int x = 0;
	int y = 0;
	for (int i = 0; i < v.size(); i++) {
		x += v[i].x;
		y += v[i].y;
		line(src, v[i], v[(i+1)%v.size()], Scalar(0, 0, 255), 3, 8);
	}

	Point center(cvRound(x / v.size()), cvRound(y / v.size()));
	circle(src, center, 3, Scalar(0, 255, 0), -1, 8, 0);

	return center;
}

float distanceee(Point p0, Point p1)  {
	return sqrt((p0.x - p1.x)*(p0.x - p1.x) + (p0.y - p1.y)*(p0.y - p1.y));
}

void MouseCapture(int event, int x, int y, int flags, void* userdata)
{
	if (event==EVENT_LBUTTONDOWN) {
		
		float min;
		for (int i = 0; i < countObjects; i++) {
			float dist = distanceee(Point(x, y), objects[i].getPosition());
			if (i == 0) { min = dist; }
			else if (min > dist) {
				min = dist;
				minId = i;
			}
		}		
	}
	
}

int main(int argc, char** argv) {
	
	initialize();
	calibrate();	

	time_t start, end;
	int counter = 0;
	double sec;
	double fps;
	
	bool isInside = false;

	if (source) { cap.read(src); }
	Mat aimArea = Mat::zeros(src.size(), CV_8UC3);
	cv::Size img_size = src.size();
	Point center = Point(img_size.width / 2, img_size.height / 2);
	int radius = 50;
	circle(aimArea, center, radius, Scalar(0, 0, 255), 1, 8, 0);

	Mat processed[10];
	Mat fullProcessed;
	bool showProcessed = false;
	bool showTracking = false;	

	while (true) {

		countObjects = 0;

		if (source) { if (!cap.read(src)) { printf("Cannot read a frame from video stream\n"); } }
		else { src = imread(imageFile); }

		if (counter == 0) { time(&start); }
		
		future<Mat> t_hsvthreshold[10];		
		for (int i = 0; i < countColors; i++) {	t_hsvthreshold[i] = async(hsvthreshold, src, colors[i]); }
		for (int i = 0; i < countColors; i++) {	t_hsvthreshold[i].wait(); }
		for (int i = 0; i < countColors; i++) {	
			processed[i] = t_hsvthreshold[i].get();
			imshow("Color - " + to_string(i), processed[i]);
			
			GaussianBlur(processed[i], processed[i], Size(3, 3), 2, 2);
			imshow("Gauss - " + to_string(i), processed[i]);

			Canny(processed[i], processed[i], 60, 180, 3);
			imshow("Canny - " + to_string(i), processed[i]);
		}


		future<vector<Vec3f>> t_circles[10];
		for (int i = 0; i < countColors; i++) { t_circles[i] = async(findCircles, processed[i]); }
		for (int i = 0; i < countColors; i++) {	t_circles[i].wait(); }

		vector<Vec3f> circles[10];

		int textHeight = img_size.height - 10;
		for (int i = 0; i < countColors; i++) {

			circles[i] = t_circles[i].get();			
			
			for (size_t j = 0; j < circles[i].size(); j++) {				
				drawCircle(src, circles[i][j]);				
				addObject(Point(cvRound(circles[i][j][0]), cvRound(circles[i][j][1])), Shape::CIRCLE, colors[i].getName());
			}
			
			

			vector<vector<Point>> contours;
			findContours(processed[i], contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			vector<Point> approx;

			for (int j = 0; j < contours.size(); j++) {
				approxPolyDP(Mat(contours[j]), approx, arcLength(Mat(contours[j]), true)*0.02, true);

				if (fabs(contourArea(contours[j])) < 100 || !isContourConvex(approx)) continue;
				if (approx.size() == 3)	{ addObject(drawPolygon(src, approx), Shape::TRIANGLE, colors[i].getName()); }
				if (approx.size() == 4) {
										
					vector<double> cos;
					for (int k = 2; k < approx.size() + 1; k++) { cos.push_back(angle(approx[k%approx.size()], approx[k-2], approx[k-1])); }
					sort(cos.begin(), cos.end());

					if (cos.front() >= -0.3 && cos.back() <= 0.3) {
					//if (true) {
						addObject(drawPolygon(src, approx), Shape::RECTANGLE, colors[i].getName());
					}
					
				}

			}

		}


		for (int i = 0; i < countObjects; i++) {

			String temp = "";
			if (i == minId) { temp += " > "; }
			temp+= ShapeString[objects[i].getShape()];
			temp += " - " ;
			temp += objects[i].getColor();
			temp += " - ";
			temp += "(" + to_string(objects[i].getPosition().x) + "," + to_string(objects[i].getPosition().y) + ")";

			putText(src, temp, Point(10, textHeight), FONT_HERSHEY_COMPLEX_SMALL, 0.6, Scalar(0, 0, 0), 1, CV_AA);
			textHeight = textHeight - 12;

		}

		if (showProcessed) {
			for (int i = 1; i < countColors; i++) {
				imshow(colors[i].getName(), processed[i]);
			}
		}

	
		/*for (int i = 0; i < countObjects; i++) {
			if (flags[i]) {
				objects[i].setOriginal(original);
				String message;

				if (objects[i].exists()) {		

					isInside = pow(objects[i].getCentroid().x - center.x, 2) + pow(objects[i].getCentroid().y - center.y, 2) <= pow(radius, 2);
					if (!isInside) {						
						int x, y;
						
						if (objects[i].getCentroid().y > center.y + radius) { y = camY + 1;	}
						if (objects[i].getCentroid().y < center.y - radius) { y = camY - 1;	}
						if (objects[i].getCentroid().x > center.x + radius) { x = camX - 1;	}
						if (objects[i].getCentroid().x < center.x - radius) { x = camX + 1;	}
							
						move_cam(x, y);
					}


					if (INCLUDE_AIM) {
						int aimSize = 30;

						Point ver_b = objects[i].getCentroid();
						ver_b.y = ver_b.y - aimSize;
						Point ver_e = objects[i].getCentroid();
						ver_e.y = ver_e.y + aimSize;
						Point hor_b = objects[i].getCentroid();
						hor_b.x = hor_b.x - aimSize;
						Point hor_e = objects[i].getCentroid();
						hor_e.x = hor_e.x + aimSize;
						line(outerLayer, hor_b, hor_e, Scalar(0, 0, 255), 2, 8);
						line(outerLayer, ver_b, ver_e, Scalar(0, 0, 255), 2, 8);
					}

					if (INCLUDE_DISTANCE) {
						line(outerLayer, objects[i].getCentroid(), center, Scalar(0, 0, 255), 2, 8);
					}
					
					message = objects[i].toString();					
					

				}
				else {
					message = objects[i].getName() + " not found";
				}


				if (INCLUDE_TEXT) {
					putText(outerLayer, message, Point(0, textHeight), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255, 255, 255), 1, CV_AA);
					textHeight -= 30;
				}
				if (PROCESSED) {
					imshow("Thresholded Image for object " + objects[i].getName(), objects[i].getProcessed());
				}
				if (TRACKING) {
					imshow("Tracking for object " + objects[i].getName(), objects[i].getTracking());
				}				
			}
		}*/


//		original = original + outerLayer + aimArea;

		

		/*Mat fprocessed = processed[0];

		for (int i = 0; i < countColors; i++) {
			fprocessed = fprocessed + processed[i];
		}		

		imshow("Processed", fprocessed);*/

		time(&end);
		counter++;
		sec = difftime(end, start);
		fps = counter / sec;
		

		string s_fps;

		if (counter > 30) {
			ostringstream out;
			out << setprecision(3) << fps;
			s_fps = out.str();
		}
		if (counter == (INT_MAX - 1000)) { counter = 0;	}

		
		putText(src, s_fps, Point(src.size().width - 70, 20), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0, 0, 0), 1, CV_AA);
		imshow("Original", src);
		setMouseCallback("Original", MouseCapture, NULL);

		switch (waitKey(10)) {	
			case 116:
				showTracking = !showTracking;
				break;
			case 112:
				showProcessed = !showProcessed;
				break;
			case 27:
					destroyAllWindows();
				for (int i = 0; i < countColors; i++) { colors[i].save(); }
			return 0;
		}
	}

	system("pause");
	return 0;
}