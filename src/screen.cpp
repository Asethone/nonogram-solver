#include "screen.h"

#include <opencv2/opencv.hpp>

#include "controls.h"

Screen::Screen() {
    update();
}

void Screen::update() {
    adb::takeScreenshot();
    screen_image_ = Image::fromScreenshot();
}

void Screen::captureAnswer(int width, int height, bool is_colored) {
    screen_image_.extractAnswer().saveToBitmap(width, height);
}

void Screen::paint(int width, int height, bool is_colored) {
    Image nonogram = screen_image_.extractNonogram();

    cv::Scalar bg_color;
    Image grid = nonogram.extractGrid(bg_color);
    std::println("background color is {}, {}, {}", bg_color[0], bg_color[1], bg_color[2]);

    const double cell_width = (double)grid.mat_.cols / width;
    const double cell_height = (double)grid.mat_.rows / height;
    double x = grid.rect_.x + cell_width / 2;
    double y = grid.rect_.y + cell_height / 2;
    if (is_colored) {
        // TODO
    } else {
        ControlSession ctrl(screen_image_.mat_.cols, screen_image_.mat_.rows);
        cv::Mat answer = Image::fromBitmap(is_colored).mat_;
        // for debugging
        cv::Mat debug = screen_image_.mat_.clone();
        // start painting
        for (int row = 0; row < height; row++) {
            x = grid.rect_.x + cell_width / 2;
            for (int col = 0; col < width; col++, x += cell_width) {
                if (answer.at<uchar>(cv::Point(col, row)) < 230) {
                    ctrl.tap(x, y);
                    cv::drawMarker(debug, {(int)x, (int)y}, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, cell_width / 2);
                }
            }
            y += cell_height;
        }
        cv::imwrite("grid.png", debug);
    }
}
