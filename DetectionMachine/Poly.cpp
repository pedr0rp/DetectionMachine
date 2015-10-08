#include "Poly.h"

void Poly::setV(std::vector<cv::Point> value) { v = value; }

std::vector<cv::Point> Poly::getV() { return v; }