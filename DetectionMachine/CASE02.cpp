#include "CASE02.h"


void CASE02::mouseCapture(int event, int x, int y, int flags, void* param) {
	CASE02* ptr = (CASE02*)param;
	if (event == cv::EVENT_LBUTTONDOWN) {
		float min;
		for (int i = 0; i < ptr->objects.size(); i++) {
			float dist = Util::distance(cv::Point(x, y), ptr->objects[i]->getPosition());
			if (i == 0) {
				ptr->index = i;
				min = dist;
			}
			else if (min > dist) {
				min = dist;
				ptr->index = i;
			}
		}

	}
}

bool CASE02::init() {

	bool ready = true;

	printf("Time: ");
	scanf("%d", &time);

	printf("Source: ");
	scanf("%d", &source);

	if (source) {
		printf("\tWeb cam");
		Util::setCam(CAM);
		if (Util::camIsOpened()) {
			printf(" connected\n");
		}
		else {
			printf(" disconnected\n");
			return 0;
			ready = false;
		}
	}
	else {
		bool fileFound = false;
		do {
			printf("\tFilename: ");
			scanf("%s", &imageFile);
			std::ifstream f(imageFile);
			fileFound = f.good();
			f.close();

		} while (!fileFound);

		Util::setImage(imageFile);

	}
	return ready;
}

void CASE02::calibrate() {

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
			}
			else {
				printf("ERROR!\n");
			}
		}
	}
	else {
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
				src = Util::readImage();
				thresholded = Util::threshold(src, colors[i]);

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

	printf("Key color: ");
	scanf("%d", &temp);
	keyColor = temp - 1;

}

int CASE02::start() {
	init();
	calibrate();

	if (time != -1) {
		std::time(&_start);
	}

	cv::Mat original;
	cv::Mat resized;
	cv::Mat processed[MAX_COLOR];

	original = Util::readImage();
	resized = Util::resize(original);
	cv::cvtColor(resized, resized, cv::COLOR_BGR2HSV);

	cv::Size size = resized.size();
	cv::Point center = cv::Point(original.size().width / 2, original.size().height / 2);
	int radiusAim = 60;
	bool inside;

	bool flag = true;


	if (false) {

		cv::namedWindow("Hough", cv::WINDOW_NORMAL);

		cv::createTrackbar("A", "Hough", &Util::houghA, 500);
		cv::createTrackbar("B", "Hough", &Util::houghB, 500);
	}


	while (flag) {

		if (Util::houghA == 0) { Util::houghA = 1; }
		if (Util::houghB == 0) { Util::houghB = 1; }

		framerate.start();
		original = Util::readImage();
		int textHeight = original.size().height - 10;

		resized = Util::resize(original);
		spaces.clear();
		objects.clear();

		std::vector<std::future<cv::Mat>> t_preprocessing(MAX_COLOR);

		for (int i = 0; i < colorCount; i++) {
			HSV color = colors[i];
			t_preprocessing[i] = std::async(std::launch::async, [resized, color] { return Util::preprocessing(resized, color); });
		}

		for (int i = 0; i < colorCount; i++) { t_preprocessing[i].wait(); }
		for (int i = 0; i < colorCount; i++) { processed[i] = t_preprocessing[i].get(); }

		if (SHOW_PROCESSED) {
			for (int i = 0; i < colorCount; i++) {
				imshow("Color " + std::to_string(i) + " processed", processed[i]);
			}
		}

		std::vector<std::future<std::vector<Circle*>>> t_circles(MAX_COLOR);
		for (int i = 0; i < colorCount; i++) {
			cv::Mat temp = processed[i];
			HSV color = colors[i];
			t_circles[i] = std::async(std::launch::async, [temp, color] { return Util::findCircles(temp, color); });
		}

		for (int i = 0; i < colorCount; i++) {
			t_circles[i].wait();
		}

		std::vector<std::future<std::vector<Poly*>>> t_poly(MAX_COLOR);
		for (int i = 0; i < colorCount; i++) {
			cv::Mat temp = processed[i];
			HSV color = colors[i];
			t_poly[i] = std::async(std::launch::async, [temp, color] { return Util::findPoly(temp, color); });
		}

		for (int i = 0; i < colorCount; i++) {
			t_poly[i].wait();
		}

		for (int i = 0; i < colorCount; i++) {			
			std::vector<Circle*> circles = t_circles[i].get();
			if (i == keyColor) {
				spaces.insert(spaces.end(), circles.begin(), circles.end());
			}
			else {
				objects.insert(objects.end(), circles.begin(), circles.end());
			}

			std::vector<Poly*> polys = t_poly[i].get();
			for (int j = 0; j < circles.size(); j++) {
				for (int k = 0; k < polys.size(); k++) {
					if (Util::distance(polys[k]->getPosition(), circles[j]->getPosition()) < 30) {
						polys.erase(polys.begin() + k);
					}
				}
			}

			if (i == keyColor) {
				spaces.insert(spaces.end(), polys.begin(), polys.end());
			}
			else {
				objects.insert(objects.end(), polys.begin(), polys.end());
			}

			
		}		

		float tolerance = 0.25;

		for (int i = 0; i < spaces.size(); i++) {
			for (int j = 0; j < objects.size(); j++) {
				if (spaces[i]->getShape() == objects[j]->getShape()) {
					if (spaces[i]->getShape() == Shape::CIRCLE) {
						Circle* space = dynamic_cast<Circle*>(spaces[i]);
						Circle* object = dynamic_cast<Circle*>(objects[j]);

						if (space->getRadius()*(1+tolerance)>= object->getRadius() && space->getRadius()* (1 - tolerance)<= object->getRadius()) {
							line(original, space->getPosition(), object->getPosition(), cv::Scalar(255, 0, 255), 2, 8);
						}
					}
					else {
						Poly* space = dynamic_cast<Poly*>(spaces[i]);
						Poly* object = dynamic_cast<Poly*>(objects[j]);

						std::vector<int> ts;
						std::vector<int> to;
						for (int k = 0; k < space->getV().size(); k++) {
							ts.push_back(Util::distance(space->getV()[k], space->getV()[(k + 1) % space->getV().size()]));
							to.push_back(Util::distance(object->getV()[k], object->getV()[(k + 1) % object->getV().size()]));							
						}

						sort(to.begin(), to.end());
						sort(ts.begin(), ts.end());

						bool flag = true;
						int k = 0;
						while (flag && k < ts.size()) {
							flag = ts[k] *(1+tolerance) >= to[k] && ts[k]*(1-tolerance) <= to[k];
							k++;
						}

						if (flag) {
							line(original, space->getPosition(), object->getPosition(), cv::Scalar(255, 0, 255), 2, 8);							
						}

					}
				}
				
			}
		}

		for (int i = 0; i < objects.size(); i++) {
			//Util::drawObject(original, objects[i]);
		}

		std::string temp = "";
		temp += "Spaces: ";
		temp += std::to_string(spaces.size());
		temp += " Objects: ";
		temp += std::to_string(objects.size());

		//putText(original, temp, cv::Point(10, textHeight), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6, cv::Scalar(255, 255, 255), 1, CV_AA);
		
		putText(original, framerate.end(), cv::Point(original.size().width - 70, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(255, 255, 255), 1, CV_AA);

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

