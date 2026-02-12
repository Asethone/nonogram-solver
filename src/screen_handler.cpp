#include "screen_handler.h"

#include <opencv2/opencv.hpp>

void ScreenHandler::readAnswer() {
    cv::Mat image = cv::imread("screenshot.png");
    // init sizes
    const int width = image.cols;
    const int height = image.rows;
    // TODO: these numbers are different for various devices
    constexpr int left_margin = 50;
    constexpr int right_margin = 50;
    constexpr int bottom_margin = 182;
    constexpr int top_margin = 289;
    // remove frame borders, leave only canvas to work with
    cv::Rect roi(
        left_margin,
        top_margin,
        width - left_margin - right_margin,
        height - top_margin - bottom_margin
    );
    cv::Mat cropped = image(roi);
    // covert to grayscale
    cv::Mat gray;
    cv::cvtColor(cropped, gray, cv::COLOR_BGR2GRAY);
    // mask
    cv::Mat mask;
    cv::threshold(gray, mask, 230, 255, cv::THRESH_BINARY_INV);
    // find bounding box of the picture
    cv::Rect bounding_box = cv::boundingRect(mask);
    // crop the picture
    cv::Mat picture = cropped(bounding_box);
    cv::imwrite("picture.png", picture);
}
