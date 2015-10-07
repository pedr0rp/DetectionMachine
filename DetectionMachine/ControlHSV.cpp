#include "opencv2/imgproc/imgproc.hpp",
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

class ControlHSV {
private:
	string name;
	int id = -1;

	int lowH = 0;
	int highH = 179;

	int lowS = 0;
	int highS = 255;

	int lowV = 0;
	int highV = 255;

public:
	int ControlHSV::COUNT = 0;

	string getWindowName() { return "Color " + name; }

	void show() {

		namedWindow(getWindowName(), WINDOW_NORMAL);

		createTrackbar("LowH", getWindowName(), &lowH, 179); //Hue (0 - 179)
		createTrackbar("HighH", getWindowName(), &highH, 179);

		createTrackbar("LowS", getWindowName(), &lowS, 255); //Saturation (0 - 255)
		createTrackbar("HighS", getWindowName(), &highS, 255);

		createTrackbar("LowV", getWindowName(), &lowV, 255);//Value (0 - 255)
		createTrackbar("HighV", getWindowName(), &highV, 255);
	}

	Scalar getLow() {
		return Scalar(lowH, lowS, lowV);
	}
	Scalar getHigh(){
		return Scalar(highH, highS, highV);
	}

	void setLow(int lowH, int lowS, int lowV) {
		ControlHSV::lowH = lowH;
		ControlHSV::lowS = lowS;
		ControlHSV::lowV = lowV;
	}

	void setHigh(int highH, int highS, int highV) {
		ControlHSV::highH = highH;
		ControlHSV::highS = highS;
		ControlHSV::highV = highV;
	}

	void setName(string value) {
		name = value;
	}

	string getName() {
		return name;
	}

	void save() {
		std::ofstream o("colors\\" + name + ".color");
		String content = "";
		content += to_string((int)getLow()[0]) + ";";
		content += to_string((int)getLow()[1]) + ";";
		content += to_string((int)getLow()[2]) + ";";
		content += to_string((int)getHigh()[0]) + ";";
		content += to_string((int)getHigh()[1]) + ";";
		content += to_string((int)getHigh()[2]) + ";";

		o << content << std::endl;
	}

	bool load() {
		ifstream infile;

		char data[100];
		infile.open("colors\\" + name + ".color");
		if (infile.is_open()) {
			infile >> data;
			infile.close();

			String content(data);
			vector<String> v = splitString(content, ';');
			setLow(stoi(v[0]), stoi(v[1]), stoi(v[2]));
			setHigh(stoi(v[3]), stoi(v[4]), stoi(v[5]));

			return true;
		}
		else {
			return false;
		}
	}

	vector<String> splitString(const String &s, char delim) {
		stringstream ss(s);
		String item;
		vector<String> tokens;
		while (getline(ss, item, delim)) {
			tokens.push_back(item);
		}
		return tokens;
	}

};
