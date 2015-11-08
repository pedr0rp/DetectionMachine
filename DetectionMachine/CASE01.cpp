#include "CASE01.h"


void CASE01::mouseCapture(int event, int x, int y, int flags, void* param) {
	CASE01* ptr = (CASE01*)param;
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

bool CASE01::init() {

	bool ready = true;

	printf("Arduino");
	if (arduino.isConnected()) {
		printf(" connected");
		arduino.left();
	}
	else {
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
		Util::setCam(CAM);
		/*if (cap.isOpened()) {
			printf(" connected\n");
		}
		else {
			printf("\t disconnected\n");
			return 0;
			ready = false;
		}*/
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

void CASE01::calibrate() {

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
}

int CASE01::start() {
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

	while (flag) {
		framerate.start();
		original = Util::readImage();
		int textHeight = original.size().height - 10;

		resized = Util::resize(original);
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
			Util::drawObject(original, objects[i]);

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