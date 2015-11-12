#include "CASE03.h"


void CASE03::mouseCapture(int event, int x, int y, int flags, void* param) {
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		CASE03* ptr = (CASE03*)param;
		ptr->ruler.push_back(cv::Point(x, y));
	}
}

bool CASE03::init() {

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

void CASE03::calibrate() {

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

	printf("\tCalibrating ruler...\n");
	cv::namedWindow("Ruler", cv::WINDOW_AUTOSIZE);
	cv::setMouseCallback("Ruler", mouseCapture, (void *)this);
	bool flag = false;
	int click = 0;
	while (click < 2) {

		if (!flag) {
			printf("\tClick #%d\n", (click + 1));
			flag = true;
		}
		
		if (ruler.size() > click)
		{
			click++;
			flag = false;
		}

		cv::Mat src = Util::readImage();
		imshow("Ruler", src);
		cv::waitKey(1);
	}

	printf("Ruler size: ");
	scanf("%f", &size);
	ratio = size / Util::distance(ruler[0], ruler[1]);
	cv::destroyAllWindows();
	
	printf("Ratio %6.4lf\n", ratio);

}

int CASE03::start() {
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
			for (int j = 0; j < circles.size(); j++) {
				for (int k = 0; k < polys.size(); k++) {
					if (Util::distance(polys[k]->getPosition(), circles[j]->getPosition()) < 10) {
						polys.erase(polys.begin() + k);

					}
				}
			}

			objects.insert(objects.end(), polys.begin(), polys.end());
		}	

		for (int i = 0; i < objects.size(); i++) {

			Util::drawObject(original, objects[i]);

			if (objects[i]->getShape() == Shape::CIRCLE) {
				Circle* circle = dynamic_cast<Circle*>(objects[i]);
				putText(original, std::to_string(circle->getRadius()*ratio), circle->getPosition(), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6, cv::Scalar(255, 255, 255), 1, CV_AA);
			}
			else {
				Poly* poly = dynamic_cast<Poly*>(objects[i]);

				for (int k = 0; k < poly->getV().size(); k++) {

					double size = Util::distance(poly->getV()[k], poly->getV()[(k + 1) % poly->getV().size()])*ratio;
					std::stringstream stream;
					stream << std::fixed << std::setprecision(2) <<size;					

					putText(original, 
						stream.str(),
						cv::Point(
							(poly->getV()[k].x + poly->getV()[(k + 1) % poly->getV().size()].x)/2,
							(poly->getV()[k].y + poly->getV()[(k + 1) % poly->getV().size()].y)/2
							),
						cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6, cv::Scalar(255, 255, 255), 1, CV_AA);
				}
			}

			

			std::string temp = "";
			temp += shapeString[objects[i]->getShape()];
			temp += " - ";
			temp += objects[i]->getColor();
			temp += " - ";
			temp += "(" + std::to_string(objects[i]->getPosition().x) + "," + std::to_string(objects[i]->getPosition().y) + ")";

			putText(original, temp, cv::Point(10, textHeight), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6, cv::Scalar(0, 0, 0), 1, CV_AA);
			textHeight = textHeight - 12;
		}

		imshow("Source", original);		

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
