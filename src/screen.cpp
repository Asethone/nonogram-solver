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

    cv::imwrite("debug.png", screen_image_.mat_(nonogram.rect_));
}
