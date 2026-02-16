#pragma once

#include <opencv2/opencv.hpp>

class Image {
public:
    Image();
    Image(const cv::Mat& mat);
    Image(const cv::Mat& mat, const cv::Rect& rect);

public:
    // takes screenshot and initialize image from it
    static Image fromScreenshot();
    // loads previously saved bitmap
    static Image fromBitmap();

    // save pixels to bitmap
    void saveToBitmap(unsigned nonogram_width, unsigned nonogram_height);
    // calculate image mask to help mask out background cells
    // if `is_inverted` is false, background colored cells are `1`
    Image getMask(int thresh = 240, bool is_inverted = false);
    // reduce noise on current mask
    Image reduceNoise();

    // retrieve answer image
    // the answer MUST be presented in full screen
    Image extractAnswer();

    // retrieve the whole nonogram grid from the screen
    // the nonogram MUST be in clear state and default position as if it's just opened for the first time
    Image extractNonogram();
    // retrieve the grid of the nonogram (the actual drawing area)
    // as well as its background color
    // MUST be called on the result of `extractNonogram()` function
    Image extractGrid(cv::Scalar& bg_color);

public:
    // opencv matrix
    cv::Mat mat_;
    // if not empty, matches the rect of another image that contains this image
    cv::Rect rect_;
};
