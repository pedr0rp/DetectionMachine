#include "Util.h"

const float Util::resizeRatio = 0.5;

cv::VideoCapture Util::cap;
char Util::imageFile[20];
int Util::CAM = -1;

bool Util::TRACKING = false;
bool Util::SHOW_PROCESSED = true;
bool Util::INCLUDE_TEXT = true;
bool Util::INCLUDE_AIM = false;
bool Util::INCLUDE_DISTANCE = true;

cv::Mat Util::threshold(cv::Mat src, HSV color) {

	cv::Mat thresholded;
	cv::inRange(src, color.getLow(), color.getHigh(), thresholded);

	int n = 3;
	//n = n*resizeRatio;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(n, n));

	cv::erode(thresholded, thresholded, element);
	cv::dilate(thresholded, thresholded, element);

	cv::dilate(thresholded, thresholded, element);
	cv::erode(thresholded, thresholded, element);

	return thresholded;

}

cv::Mat Util::preprocessing(cv::Mat src, HSV color) {

	src = threshold(src, color);	

	int n = 3;
	//n = n*resizeRatio;
	cv::GaussianBlur(src, src, cv::Size(n, n), 1, 1);
	cv::Canny(src, src, 60, 60*3, 3);

	return src;
}

std::vector<Circle*> Util::findCircles(cv::Mat src, HSV color) {
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(src, circles, CV_HOUGH_GRADIENT, 2, src.rows / 8, 90, 60, 10, 0);
	
	std::vector<Circle*> objects;

	for (size_t i = 0; i < circles.size(); i++) {				
		Circle* circle = new Circle;
		circle->setPosition(cv::Point(cvRound(circles[i][0]*(1/resizeRatio)), cvRound(circles[i][1]*(1/resizeRatio))));
		circle->setShape(Shape::CIRCLE);
		circle->setColor(color.getName());
		circle->setRadius(cvRound(circles[i][2])*(1/resizeRatio));
		objects.push_back(circle);
	}

	return objects;
}

 double Util::angle(cv::Point p1, cv::Point p2, cv::Point p0)
{
	double dx1 = p1.x - p0.x;
	double dy1 = p1.y - p0.y;
	double dx2 = p2.x - p0.x;
	double dy2 = p2.y - p0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

 std::vector<cv::Point> Util::removeNear(std::vector<cv::Point> src) {
	 typedef std::vector<std::vector<int> > Matrix;
	 typedef std::vector<int> Row;

	 const int N = src.size();
	 Matrix d;

	 for (int j = 0; j < N; ++j)	{
		 Row row(N);
		 for (int k = 0; k < N; ++k)	{
			 row[k] = distance(src[j], src[k]);
		 }
		 d.push_back(row); 
	 }

	 std::vector<int> notInclude;
	 int maxPixel = 6;

	 for (int j = 0; j < src.size(); j++) {
		 if (std::find(notInclude.begin(), notInclude.end(), j) == notInclude.end()) {
			 for (int k = 0; k < src.size(); k++) {
				 if (d[j][k] > 0 && d[j][k] < maxPixel*(1 / resizeRatio)) {
					 notInclude.push_back(k);
				 }
			 }
		 }
	 }

	 for (int j = 0; j < src.size(); j++) {
		  for (int k = 0; k < src.size(); k++) {
			  printf("%03d\t",d[j][k]);			 
		 }
		  printf("\n");
	 }

	 std::vector<cv::Point> v;
	 for (int j = 0; j < src.size(); j++) {
		 if (std::find(notInclude.begin(), notInclude.end(), j) == notInclude.end()) {
			 v.push_back(src[j]);
		 }
	 }

	 return v;
 }

std::vector<Poly*> Util::findPoly(cv::Mat src, HSV color) {

	std::vector<Poly*> objects;
	
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(src, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	
	std::vector<cv::Point> approx;
	
	for (int i = 0; i < contours.size(); i++) {

		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);
		if (fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx)) continue;

		std::vector<cv::Point> v = removeNear(approx);
		//std::vector<cv::Point> v = approx;

		if (v.size() == 3)	{
			Poly* object = new Poly;
			object->setColor(color.getName());
			int x, y;
			x = y = 0;
			for (int i = 0; i < v.size(); i++) {
				v[i].x = v[i].x*(1 / resizeRatio);
				v[i].y = v[i].y*(1 / resizeRatio);
				x += v[i].x;
				y += v[i].y;
			}
			object->setV(v);
			object->setPosition(cv::Point(cvRound(x / v.size()), cvRound(y / v.size())));
			object->setShape(Shape::TRIANGLE);
			objects.push_back(object);
		}
		if (v.size() == 4) {
											
			std::vector<double> cos;
			for (int j = 2; j < v.size() + 1; j++) { 
				cos.push_back(angle(v[j%v.size()], v[j-2], v[j-1])); 
			}
			sort(cos.begin(), cos.end());	
			
			if (cos.front() >= -0.3 && cos.back() <= 0.3) {
				Poly* object = new Poly;
				object->setColor(color.getName());
				int x, y;
				x = y = 0;
				for (int i = 0; i < v.size(); i++) {
					v[i].x = v[i].x*(1 / resizeRatio);
					v[i].y = v[i].y*(1 / resizeRatio);
					x += v[i].x;
					y += v[i].y;
				}
				object->setV(v);
				object->setPosition(cv::Point(cvRound(x / v.size()), cvRound(y / v.size())));
				object->setShape(Shape::RECTANGLE);

				objects.push_back(object);
			}						
		}	

		if (v.size() > 4) {
			Poly* object = new Poly;
			object->setColor(color.getName());
			int x, y;
			x = y = 0;
			for (int i = 0; i < v.size(); i++) {
				v[i].x = v[i].x*(1 / resizeRatio);
				v[i].y = v[i].y*(1 / resizeRatio);
				x += v[i].x;
				y += v[i].y;
			}
			object->setV(v);
			object->setPosition(cv::Point(cvRound(x / v.size()), cvRound(y / v.size())));
			object->setShape(Shape::OTHER);

			objects.push_back(object);
		}
	}

	return objects;
}

void Util::drawCircle(cv::Mat &src, Circle* circle) {
	cv::circle(src, circle->getPosition(), circle->getRadius(), cv::Scalar(0, 0, 255), 2, 8);
}

void Util::drawPoly(cv::Mat &src, Poly* poly) {
	for (int i = 0; i < poly->getV().size(); i++) {
		line(src, poly->getV()[i], poly->getV()[(i + 1) % poly->getV().size()], cv::Scalar(0, 0, 255), 2, 8);
	}
}

void Util::drawObject(cv::Mat &src, Object* object) {
	
	cv::circle(src, object->getPosition(), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
	switch (object->getShape()) {
		case Shape::CIRCLE:
			drawCircle(src, dynamic_cast<Circle*>(object));
			break;
		case Shape::RECTANGLE:
		case Shape::TRIANGLE:
		case Shape::OTHER:
			drawPoly(src, dynamic_cast<Poly*>(object));
			break;
	}	
}

float Util::distance(cv::Point p0, cv::Point p1)  {
	 return sqrt((p0.x - p1.x)*(p0.x - p1.x) + (p0.y - p1.y)*(p0.y - p1.y));
}

cv::Mat Util::resize(cv::Mat src) {
	cv::Size newSize = cv::Size(src.cols*resizeRatio, src.rows*resizeRatio);
	cv::resize(src, src, newSize, 0, 0, cv::INTER_LINEAR);
 	return src;
}

void Util::setCam(int value) {
	CAM = value;
	cap = cv::VideoCapture(CAM);
}

void Util::setImage(char value[fileSize]) {
	strncpy(imageFile, value, fileSize);
}

cv::Mat Util::readImage() {
	cv::Mat src;

	if (CAM != -1) {
		if (!cap.read(src)) {
			printf("Cannot read a frame from video stream\n");
		}
	}
	else {
		src = cv::imread(imageFile);
	}

	return src;
}