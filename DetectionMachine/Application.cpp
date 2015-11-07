#include "Application.h"

const float Application::resizeRatio = 0.5;

bool Application::init() {

	bool ready = true;

	printf("Arduino");
	if (arduino.isConnected()) {
		printf(" connected");
		arduino.left();
	} else {
		printf(" disconnected");
		ready = false;
	}
	
	printf("\n\n");

	printf("Time: ");
	scanf("%d", &time);

	printf("Source: ");
	scanf("%d", &source);

	if (source) {		
		printf("\tWeb cam");
		if (cap.isOpened()) {
			printf(" connected\n");
		} else {
			printf("\t disconnected\n");
			return 0;
			ready = false;
		}
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
			
			while (!next) {		
				src = readImage();
				thresholded = threshold(src, colors[i]);				

				imshow("Original", src);
				imshow("Thresholded", thresholded);

				switch (cv::waitKey(10)) {
				case 27:
					printf("OK!\n");
					cv::destroyAllWindows();
					colors[i].save();
					next = true;
					break;
				}
			}
		}
	}
}

cv::Mat Application::threshold(cv::Mat src, HSV color) {

	cv::Mat thresholded;

	cv::cvtColor(src, thresholded, cv::COLOR_BGR2HSV);
	cv::inRange(thresholded, color.getLow(), color.getHigh(), thresholded);

	int n = 3;
	//n = n*resizeRatio;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(n, n));

	cv::erode(thresholded, thresholded, element);
	cv::dilate(thresholded, thresholded, element);

	cv::dilate(thresholded, thresholded, element);
	cv::erode(thresholded, thresholded, element);

	return thresholded;

}

cv::Mat Application::preprocessing(cv::Mat src, HSV color) {

	src = threshold(src, color);	

	int n = 3;
	//n = n*resizeRatio;
	cv::GaussianBlur(src, src, cv::Size(n, n), 1, 1);
	cv::Canny(src, src, 60, 60*3, 3);

	return src;
}

std::vector<Circle*> Application::findCircles(cv::Mat src, HSV color) {
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

 double Application::angle(cv::Point p1, cv::Point p2, cv::Point p0)
{
	double dx1 = p1.x - p0.x;
	double dy1 = p1.y - p0.y;
	double dx2 = p2.x - p0.x;
	double dy2 = p2.y - p0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

 std::vector<cv::Point> Application::removeNear(std::vector<cv::Point> src) {
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

	 for (int j = 0; j < src.size(); j++) {
		 if (std::find(notInclude.begin(), notInclude.end(), j) == notInclude.end()) {
			 for (int k = 0; k < src.size(); k++) {
				 if (d[j][k] > 0 && d[j][k] < 8*(1/resizeRatio)) {
					 notInclude.push_back(k);
				 }
			 }
		 }
	 }

	 /*for (int j = 0; j < src.size(); j++) {
		  for (int k = 0; k < src.size(); k++) {
			  printf("%03d\t",d[j][k]);			 
		 }
		  printf("\n");
	 }*/

	 std::vector<cv::Point> v;
	 for (int j = 0; j < src.size(); j++) {
		 if (std::find(notInclude.begin(), notInclude.end(), j) == notInclude.end()) {
			 v.push_back(src[j]);
		 }
	 }

	 return v;
 }

std::vector<Poly*> Application::findPoly(cv::Mat src, HSV color) {

	std::vector<Poly*> objects;
	
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(src, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	
	std::vector<cv::Point> approx;
	
	for (int i = 0; i < contours.size(); i++) {

		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);
		if (fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx)) continue;

		std::vector<cv::Point> v = removeNear(approx);

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

		/*if (v.size() > 4) {
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
		}*/
	}

	return objects;
}

void Application::drawCircle(cv::Mat &src, Circle* circle) {
	cv::circle(src, circle->getPosition(), circle->getRadius(), cv::Scalar(0, 0, 255), 2, 8);
}

void Application::drawPoly(cv::Mat &src, Poly* poly) {
	for (int i = 0; i < poly->getV().size(); i++) {
		line(src, poly->getV()[i], poly->getV()[(i + 1) % poly->getV().size()], cv::Scalar(0, 0, 255), 2, 8);
	}
}

void Application::drawObject(cv::Mat &src, Object* object) {
	
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

cv::Mat Application::readImage() {
	cv::Mat src;

	if (source) { 
		if (!cap.read(src)) { 
			printf("Cannot read a frame from video stream\n"); 
		}
	} else { 
		src = cv::imread(imageFile); 
	}

	return src;
}

void Application::mouseCapture(int event, int x, int y, int flags, void* param) {
	Application* ptr = (Application*)param;
	if (event == cv::EVENT_LBUTTONDOWN) {	
		float min;
		for (int i = 0; i < ptr->objects.size(); i++) {
			float dist = distance(cv::Point(x, y), ptr->objects[i]->getPosition());
			if (i == 0) { 
				ptr->index = i;
				min = dist;  
			} else if (min > dist) {
				min = dist;
				ptr->index = i;
			}
		}
		
	}
}

float Application::distance(cv::Point p0, cv::Point p1)  {
	 return sqrt((p0.x - p1.x)*(p0.x - p1.x) + (p0.y - p1.y)*(p0.y - p1.y));
}

cv::Mat Application::resize(cv::Mat src) {
	cv::Size newSize = cv::Size(src.cols*resizeRatio, src.rows*resizeRatio);
	cv::resize(src, src, newSize, 0, 0, cv::INTER_LINEAR);
	return src;
}

int Application::start() {
	init();
	calibrate();

	if (time != -1) {
		std::time(&_start);
	}

	cv::Mat original;
	cv::Mat resized;
	cv::Mat processed[MAX_COLOR];

	original = readImage();
	resized = resize(original);

	cv::Size size = resized.size();
	cv::Point center = cv::Point(original.size().width / 2, original.size().height / 2);
	int radiusAim = 60;	
	bool inside;	

	bool flag = true;	

	while (flag) {
		framerate.start();
		original = readImage();		
		int textHeight = original.size().height - 10;

		resized = resize(original);		
		objects.clear();	

		std::vector<std::future<cv::Mat>> t_preprocessing(MAX_COLOR);

		for (int i = 0; i < colorCount; i++) { 
			HSV color = colors[i];
			t_preprocessing[i] = std::async(std::launch::async, [resized, color] { return preprocessing(resized, color); });
		}
		
		for (int i = 0; i < colorCount; i++) { t_preprocessing[i].wait(); }
		for (int i = 0; i < colorCount; i++) { processed[i] = t_preprocessing[i].get(); }
		
		if (SHOW_PROCESSED) {
			for (int i = 0; i < colorCount; i++) {
				imshow("Color "+std::to_string(i)+" processed", processed[i]);
			}
		}

		std::vector<std::future<std::vector<Circle*>>> t_circles(MAX_COLOR);
		for (int i = 0; i < colorCount; i++) {			
			cv::Mat temp = processed[i];
			HSV color = colors[i];
			t_circles[i] = std::async(std::launch::async, [temp, color] { return findCircles(temp, color); });
		}

		for (int i = 0; i < colorCount; i++) {
			t_circles[i].wait();			
		}

		std::vector<std::future<std::vector<Poly*>>> t_poly(MAX_COLOR);
		for (int i = 0; i < colorCount; i++) {
			cv::Mat temp = processed[i];
			HSV color = colors[i];
			t_poly[i] = std::async(std::launch::async, [temp, color] { return findPoly(temp, color); });
		}

		for (int i = 0; i < colorCount; i++) { 
			t_poly[i].wait(); 
		}
		
		for (int i = 0; i < colorCount; i++) { 				
			std::vector<Circle*> circles = t_circles[i].get();
			objects.insert(objects.end(), circles.begin(), circles.end());

			std::vector<Poly*> polys = t_poly[i].get();
			objects.insert(objects.end(), polys.begin(), polys.end());
		}

		if (TRACKING && objects.size()>0) {
			inside = pow(objects[index]->getPosition().x - center.x, 2) + pow(objects[index]->getPosition().y - center.y, 2) <= pow(radiusAim, 2);
			if (!inside) {
				int x, y;

				if (objects[index]->getPosition().y > center.y + radiusAim) { arduino.down(); }
				if (objects[index]->getPosition().y < center.y - radiusAim) { arduino.up(); }
				if (objects[index]->getPosition().x > center.x + radiusAim) { arduino.left(); }
				if (objects[index]->getPosition().x < center.x - radiusAim) { arduino.right(); }
			}
			line(original, objects[index]->getPosition(), center, cv::Scalar(0, 0, 255), 2, 8);
		}

		for (int i = 0; i < objects.size(); i++) {
			drawObject(original, objects[i]);

			std::string temp = "";
			if (i == index) { temp += " > "; }
			temp += shapeString[objects[i]->getShape()];
			temp += " - ";
			temp += objects[i]->getColor();
			temp += " - ";
			temp += "(" + std::to_string(objects[i]->getPosition().x) + "," + std::to_string(objects[i]->getPosition().y) + ")";

			putText(original, temp, cv::Point(10, textHeight), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6, cv::Scalar(0, 0, 0), 1, CV_AA);
			textHeight = textHeight - 12;
		}

		putText(original, framerate.end(), cv::Point(original.size().width - 70, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(255, 255, 255), 1, CV_AA);	
		if (INCLUDE_AIM) {
			circle(original, center, radiusAim, cv::Scalar(0, 0, 255), 1, 8, 0);
		}

		imshow("Source", original);
		cv::setMouseCallback("Source", mouseCapture, (void *)this);
		int qtd = objects.size();

		switch (cv::waitKey(1)) {			
			case 27:
				cv::destroyAllWindows();
				for (int i = 0; i < colorCount; i++) { colors[i].save(); }
			return 0;
		}

		if (time != 0) {
			flag = std::difftime(std::time(0), _start) < time; 
		}		
	}

	return 0;
}