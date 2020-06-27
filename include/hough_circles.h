#ifndef HOUGH_CIRCLES_H
#define HOUGH_CIRCLES_H

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <cstdlib>
#include <map>
#include <chrono>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <boost/filesystem.hpp>

#include <tesseract/baseapi.h>
#include "leptonica/allheaders.h"

class HoughCircles
{
public:
    HoughCircles();
    HoughCircles(const int width, const int height);
    std::string recognitionText(const std::string &pathToImage);

private:
    cv::Mat image;
    int width;
    int height;
    cv::Mat imageGray;

    std::pair<int, int> getLocalMaximums(const cv::Mat &image, const int thresh_width, const int border, const int thresh_max, const int thresh_min, const int mode);
};

#endif // HOUGH_CIRCLES_H
