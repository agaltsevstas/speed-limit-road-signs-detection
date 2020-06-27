#include <boost/filesystem.hpp>

#include "hough_circles.h"

HoughCircles::HoughCircles(const int width, const int height)
{
    this->width = width;
    this->height = height;
}

std::pair<int, int> HoughCircles::getLocalMaximums(const cv::Mat &image, const int thresh_width, const int thresh_border, const int thresh_max, const int thresh_min, const int mode)
{
    cv::Mat hist;
    cv::reduce(image, hist, mode, cv::REDUCE_AVG, CV_8UC1);
    cv::Mat temp;
    double thresh_Otsu = cv::threshold(hist, temp, 0, 255, cv::THRESH_OTSU);
    hist.setTo(0, hist < thresh_Otsu);
    cv::reduce(hist, hist, mode, cv::REDUCE_SUM, CV_32FC1);

    int elem {}, index {};
    std::vector<int> local_maxs;
    int range {};

    mode ? range = hist.rows : range = hist.cols;

    for (int i = thresh_border + 1; i < range - thresh_border - 1; ++i)
    {
        if (hist.at<float>(i) > hist.at<float>(i - 1) && hist.at<float>(i) > hist.at<float>(i + 1))
        {
            local_maxs.push_back(i);
            elem = 0;
        }
        else if (hist.at<float>(i) > hist.at<float>(i - 1) && hist.at<float>(i) == hist.at<float>(i + 1))
        {
            elem = hist.at<float>(i);
            index = i;
        }
        else if (elem > hist.at<float>(i + 1))
        {
            local_maxs.push_back(index);
            elem = 0;
        }
    }

    if(!local_maxs.empty())
    {
        for (std::vector<int>::iterator local_max = local_maxs.begin(); local_max != local_maxs.end() - 1; ++local_max)
        {
            if (*local_max + thresh_width > *(local_max + 1))
            {
                if (hist.at<float>(*local_max) < hist.at<float>(*(local_max + 1)))
                {
                    local_max = --local_maxs.erase(local_max);
                }
                else
                {
                    hist.at<float>(*(local_max + 1)) = hist.at<float>(*local_max);
                    local_max = --local_maxs.erase(local_max);
                }
            }
        }
    }

    std::pair<int, int> two_points;

    while ((thresh_min >= two_points.second - two_points.first || two_points.second - two_points.first >= thresh_max) && local_maxs.size() > 1)
    {
        std::sort(local_maxs.begin(), local_maxs.end(), std::greater<int>());
        std::rotate(local_maxs.rbegin(), local_maxs.rbegin() + 1, local_maxs.rend());
        two_points.first = *local_maxs.begin();
        two_points.second = *(local_maxs.begin() + 1);
        local_maxs.erase(local_maxs.begin());
    }

    return two_points;
}

std::string HoughCircles::recognitionText(const std::string &pathToImage)
{
    std::cout << "Путь к изображению: " << pathToImage << std::endl;

    image = cv::imread(pathToImage);

    CV_Assert(!image.empty());

    cv::resize(image, image, cv::Size(width, height));

    cv::cvtColor(image, imageGray, cv::COLOR_RGB2GRAY);

    double mean_gray = cv::mean(imageGray)[0];
    std::cout << "mean_gray: " << mean_gray << std::endl;

    const int thresh_dp = 3;
    std::vector<std::string> signs = {"5", "10", "20", "30", "40", "50", "60", "70", "80", "90", "100"};
    std::vector<cv::Vec3f> circles;
    
    for (size_t i = 1; i <= thresh_dp; ++i)
    {
        cv::HoughCircles(imageGray, circles, cv::HOUGH_GRADIENT, i, imageGray.rows, mean_gray, imageGray.rows, imageGray.rows / 4, imageGray.rows / 2);
        
        if (!circles.empty()/* && !circles[i].empty()*/)
        {
            float x = circles[0][0], y = circles[0][1], r = circles[0][2];
            std::cout << "x: " << x << " y: " << y << " r: " << r << std::endl;
            for ( size_t i = 0; i < circles.size(); i++ )
            {
                  cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
                  int radius = cvRound(circles[i][2]);
                  // circle center
                  cv::circle(image, center, 3, cv::Scalar(0,255,0), -1, 1, 0 );
                  // circle outline
                  cv::circle(image, center, radius, cv::Scalar(0,0,255), 3, 1, 0 );
            }
            if (x + r > imageGray.cols)
            {
                cv::Mat combine = cv::Mat::zeros(cv::Size(ceil(x + r - imageGray.cols), imageGray.rows), imageGray.type());
                cv::hconcat(imageGray, combine, imageGray);
            }
            if (x - r < 0)
            {
                cv::Mat combine = cv::Mat::zeros(cv::Size(ceil(r - x), imageGray.rows), imageGray.type());
                cv::hconcat(combine, imageGray, imageGray);
                x += ceil(r - x);
            }
            if (y + r > imageGray.rows)
            {
                cv::Mat combine = cv::Mat::zeros(cv::Size(imageGray.cols, ceil(y + r - imageGray.rows)), imageGray.type());
                cv::vconcat(imageGray, combine, imageGray);
            }
            if (y - r < 0)
            {
                cv::Mat combine = cv::Mat::zeros(cv::Size(imageGray.cols, ceil(r - y)), imageGray.type());
                cv::vconcat(combine, imageGray, imageGray);
                y += ceil(r - y);
            }

            cv::Mat mask;
            mask = ~cv::Mat::zeros(imageGray.rows, imageGray.cols, CV_8U);
            cv::circle(mask, cv::Point(x - 1, y - 1), r, cv::Scalar(0, 0, 0), -1, 1, 0);

            cv::Mat imageGrayMaskWhite;
            cv::bitwise_or(imageGray, mask, imageGrayMaskWhite);
            cv::Mat pixels;
            imageGrayMaskWhite.copyTo(pixels);
            pixels.setTo(1,pixels < 255);

            cv::MatIterator_<uchar> it, end;
            std::vector<uchar> imageNormalize;
            for (it = imageGrayMaskWhite.begin<uchar>(), end = imageGrayMaskWhite.end<uchar>(); it != end; ++it)
            {
                if(*it < 255)
                    imageNormalize.push_back(*it);
            }

            double min {}, max {};
            cv::minMaxLoc(imageNormalize, &min, &max);

            cv::Mat maskMin(imageGray.rows, imageGray.cols, CV_8U, max);

            cv::Mat maskMinCircleBlack;
            cv::bitwise_and(maskMin, mask, maskMinCircleBlack);

            cv::Mat imageGrayMaskBlack;
            cv::bitwise_and(imageGrayMaskWhite, ~mask, imageGrayMaskBlack);

            cv::Mat imageGrayMaskMin;
            cv::add(maskMinCircleBlack, imageGrayMaskBlack, imageGrayMaskMin);

            imageGrayMaskMin = imageGrayMaskMin(cv::Rect(x - r, y - r, 2 * r, 2 * r));

            cv::Mat imageNormalizeOnWhite, imageNormalizeBlack;
            cv::normalize(imageGrayMaskMin, imageNormalizeOnWhite, 255, 0, cv::NORM_MINMAX, -1);

            mask = ~mask(cv::Rect(x - r, y - r, 2 * r, 2 * r));
            cv::normalize(imageGrayMaskMin, imageNormalizeBlack, 255, 0, cv::NORM_MINMAX, -1, mask);
//            cv::namedWindow("imageNormalizeBlack", cv::WINDOW_NORMAL);
//            cv::imshow("imageNormalizeBlack", imageNormalizeBlack);
//            cv::namedWindow("image", cv::WINDOW_NORMAL);
//            cv::imshow("image", image);
//            cv::waitKey(0);


            std::pair<int, int> two_points;

            two_points = getLocalMaximums(imageNormalizeBlack, 0.1 * imageNormalizeBlack.rows, 0.05 * imageNormalizeBlack.rows, imageNormalizeBlack.rows / 3 * 2, imageNormalizeBlack.rows / 4, 1);
            if(two_points.first != NULL && two_points.second != NULL)
                imageNormalizeBlack = imageNormalizeBlack(cv::Rect(0, two_points.first, imageNormalizeBlack.cols, two_points.second - two_points.first));
//            boost::filesystem::path path {pathToImage};
//            cv::imwrite(path.parent_path().string() + "/" + path.stem().string() + "_saved.jpg", imageNormalizeBlack);
//            std::cout << "image: " << path.parent_path().string() + "/" + path.stem().string() + "_saved.jpg" << std::endl;

            two_points = getLocalMaximums(imageNormalizeBlack, 0.1 * imageNormalizeBlack.cols, 0.02 * imageNormalizeBlack.cols, 0.9 * imageNormalizeBlack.cols, imageNormalizeBlack.cols / 5, 0);
            if(two_points.first != NULL && two_points.second != NULL)
                imageNormalizeBlack = imageNormalizeBlack(cv::Rect(two_points.first, 0, two_points.second - two_points.first, imageNormalizeBlack.rows));

            cv::minMaxLoc(imageNormalizeBlack, &min, &max);
            cv::normalize(imageNormalizeBlack, imageNormalizeBlack, 0, 255, cv::NORM_MINMAX, -1);
            cv::resize(imageNormalizeBlack, imageNormalizeBlack, cv::Size(width, height));

//            double mean = cv::mean(imageNormalizeBlack)[0];
//            cv::threshold(imageNormalizeBlack, imageNormalizeBlack, mean, 255, cv::THRESH_BINARY);
//            double thresholdOtsu = cv::threshold(imageNormalizeBlack, imageNormalizeBlack, 0, 255, cv::THRESH_OTSU);
//            cv::resize(imageNormalizeBlack, imageNormalizeBlack, cv::Size(200, 200));
//            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
//            cv::morphologyEx(imageNormalizeBlack, imageNormalizeBlack, cv::MORPH_OPEN, kernel);

            char symbols[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

            tesseract::TessBaseAPI api;
            api.Init(NULL, "eng", tesseract::OEM_TESSERACT_ONLY);
            api.SetImage((uchar*)imageNormalizeBlack.data, imageNormalizeBlack.size().width, imageNormalizeBlack.size().height, imageNormalizeBlack.channels(), imageNormalizeBlack.step1());
            cv::normalize(imageGrayMaskMin, imageNormalizeBlack, 255, 0, cv::NORM_MINMAX, -1, mask);
            api.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
            api.SetVariable("tessedit_char_whitelist", symbols);
            api.Recognize(0);
            api.AnalyseLayout();
            std::string text { api.GetUTF8Text() };

            for(std::string::iterator symbol = text.begin(); symbol < text.end();)
            {
                isdigit(*symbol) ? ++symbol : text.erase(symbol);
            }

            for (const auto &sign: signs)
            {
                if (text == sign)
                    return text;
            }
        }        
    }

    return {};
}

