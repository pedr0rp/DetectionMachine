#include "opencv2/imgproc/imgproc.hpp",
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

enum Shape { CIRCLE = 0, RECTANGLE = 1, TRIANGLE = 2};
static const char * ShapeString[] = { "Circle", "Rectangle", "Triangle" };

class Object {
	private:		
		Point lastPosition;
		Point position; 		
		String name;
		Mat tracking;
		Shape shape;
		String color;
	public:
		
		Object(Mat src) {
			position.x = -1; 
			position.y = -1;
			lastPosition = position;
			tracking = Mat::zeros(src.size(), CV_8UC3);
		}

		void setName(String value) {
			name = value;
		}

		String getName() {
			return name;
		}

		void setPosition(Point value) {

			if (lastPosition.x != -1 && lastPosition.y != -1) {
				line(tracking, lastPosition, value, Scalar(0, 0, 255), 2, 8);
			}
			
			lastPosition = position;
			position = value;
		}

		Point getPosition() {
			return position;
		}

		Mat getTracking() {
			return tracking;
		}

		void setShape(Shape value) {
			shape = value;
		}

		Shape getShape() {
			return shape;
		}

		void setColor(string value) {
			color = value;
		}

		String getColor() {
			return color;
		}

};

