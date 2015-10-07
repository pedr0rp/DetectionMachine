#include <stdio.h>
#include <math.h>
#include <deque>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"

using namespace std;

CvSeq* getCirclesInImage(IplImage*, CvMemStorage*, IplImage*);
float eucdist(CvPoint, CvPoint);
void drawCircleAndLabel(IplImage*, float*, const char*);
bool circlesBeHomies(float*, float*);

const int MIN_IDENT = 50;
const int MAX_RAD_DIFF = 10;
const int HISTORY_SIZE = 5;
const int X_THRESH = 15;
const int Y_THRESH = 15;
const int R_THRESH = 20;
const int MATCHES_THRESH = 3;
const int HUE_BINS = 32;

int mai2n(int argc, char *argv[]) {
	CvCapture *capture = 0; //The camera
	IplImage* frame = 0; //The images you bring out of the camera

	//Open the camera
	capture = cvCaptureFromCAM(1);
	if (!capture) {
		printf("Could not connect to camera\n");
		return 1;
	}

	frame = cvQueryFrame(capture);
	//Create two output windows
	cvNamedWindow("raw_video", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("processed_video", CV_WINDOW_AUTOSIZE);

	//Used as storage element for Hough circles
	CvMemStorage* storage = cvCreateMemStorage(0);

	// Grayscale image
	IplImage* grayscaleImg = cvCreateImage(cvSize(640, 480), 8/*depth*/, 1/*channels*/);

	CvPoint track1 = { -1, -1 };
	CvPoint track2 = { -1, -1 };
	float rad1 = -1;
	float rad2 = -1;
	deque<CvSeq*> samples;
	int key = 0;
	while (key != 27 /*escape key to quit*/) {
		//Query for the next frame
		frame = cvQueryFrame(capture);
		if (!frame) break;

		deque<CvSeq*> stableCircles;
		//show the raw image in one of the windows
		cvShowImage("raw_video", frame);
		CvSeq* circles = getCirclesInImage(frame, storage, grayscaleImg);

		//Iterate through the list of circles found by cvHoughCircles()
		for (int i = 0; i < circles->total; i++) {
			int matches = 0;
			float* p = (float*)cvGetSeqElem(circles, i);
			float x = p[0];
			float y = p[1];
			float r = p[2];
			if (x - r < 0 || y - r < 0 || x + r >= frame->width || y + r >= frame->height) {
				continue;
			}
			for (int j = 0; j < samples.size(); j++) {
				CvSeq* oldSample = samples[j];
				for (int k = 0; k < oldSample->total; k++) {
					float* p2 = (float*)cvGetSeqElem(oldSample, k);
					if (circlesBeHomies(p, p2)) {
						matches++;
						break;
					}
				}
			}
			if (matches > MATCHES_THRESH) {
				cvSetImageROI(frame, cvRect(x - r, y - r, 2 * r, 2 * r));
				IplImage* copy = cvCreateImage(cvSize(2 * r, 2 * r), frame->depth, 3);
				cvCvtColor(frame, copy, CV_BGR2HSV);
				IplImage* hue = cvCreateImage(cvGetSize(copy), copy->depth, 1);
				cvCvtPixToPlane(copy, hue, 0, 0, 0);
				int hsize[] = { HUE_BINS };
				float hrange[] = { 0, 180 };
				float* range[] = { hrange };
				IplImage* hueArray[] = { hue };
				int channel[] = { 0 };
				CvHistogram* hist = cvCreateHist(1, hsize, CV_HIST_ARRAY, range, 1);
				cvCalcHist(hueArray, hist, 0, 0);
				cvNormalizeHist(hist, 1.0);
				int highestBinSeen = -1;
				float maxVal = -1;
				for (int b = 0; b<HUE_BINS; b++) {
					float binVal = cvQueryHistValue_1D(hist, b);
					if (binVal > maxVal) {
						maxVal = binVal;
						highestBinSeen = b;
					}
				}
				cvResetImageROI(frame);
				const char *color;
				switch (highestBinSeen) {
				case 2: case 3: case 4:
					color = "orange";
					break;
				case 5: case 6: case 7: case 8:
					color = "yellow";
					break;
				case 9: case 10: case 11: case 12:
				case 13: case 14: case 15: case 16:
					color = "green";
					break;
				case 17: case 18: case 19: case 20:
				case 21: case 22: case 23:
					color = "blue";
					break;
				case 24: case 25: case 26: case 27:
				case 28:
					color = "purple";
					break;
				default:
					color = "red";
				}
				char label[64];
				sprintf(label, "color: %s", color);
				drawCircleAndLabel(frame, p, label);
			}
		}
		samples.push_back(circles);
		if (samples.size() > HISTORY_SIZE) {
			samples.pop_front();
		}
		cvShowImage("processed_video", frame);

		//Get the last key that's been pressed for input
		key = cvWaitKey(1);
	}
}

CvSeq* getCirclesInImage(IplImage* frame, CvMemStorage* storage, IplImage* grayscaleImg) {
	// houghification
	// Convert to a single-channel, grayspace image
	cvCvtColor(frame, grayscaleImg, CV_BGR2GRAY);

	// Gaussian filter for less noise
	cvSmooth(grayscaleImg, grayscaleImg, CV_GAUSSIAN, 7, 9);

	//Detect the circles in the image
	CvSeq* circles = cvHoughCircles(grayscaleImg,
		storage,
		CV_HOUGH_GRADIENT,
		2,
		grayscaleImg->height / 4,
		200,
		100);
	return circles;
}

float eucdist(CvPoint c1, CvPoint c2) {
	float d = sqrt(pow((float)c1.x - c2.x, 2) + pow((float)c1.y - c2.y, 2));
	return d;
}

void drawCircleAndLabel(IplImage* frame, float* p, const char* label) {
	//Draw the circle on the original image
	//There's lots of drawing commands you can use!
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0.0, 1, 8);
	cvCircle(frame, cvPoint(cvRound(p[0]), cvRound(p[1])), cvRound(p[2]), CV_RGB(255, 0, 0), 3, 8, 0);
	cvPutText(frame, label, cvPoint(cvRound(p[0]), cvRound(p[1])), &font, CV_RGB(255, 0, 0));
}

bool circlesBeHomies(float* c1, float* c2) {
	return (abs(c1[0] - c2[0]) < X_THRESH) && (abs(c1[1] - c2[1]) < Y_THRESH) &&
		(abs(c1[2] - c2[2]) < R_THRESH);
}