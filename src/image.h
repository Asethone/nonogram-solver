#pragma once

#include <opencv2/opencv.hpp>

class Image {
public:
    Image(const cv::Mat& mat);

    // takes screenshot and initialize image from it
    static Image fromScreenshot();
    // loads previously saved bitmap
    static Image fromBitmap();

    // save pixels to bitmap
    void saveToBitmap(unsigned nonogram_width, unsigned nonogram_height);
    // retrieve answer image (the answer SHOULD be there)
    Image cropAnswer();
    // calculate image mask to help mask out background cells
    // if `is_inverted` is false, background colored cells are `1`
    Image getMask(bool is_inverted = false);
    // reduce noise on current mask
    Image reduceNoise();

private:
    cv::Mat mat_;
};
