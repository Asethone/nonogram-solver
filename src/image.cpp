#include "image.h"

#include "controls.h"

Image::Image(const cv::Mat &mat) : mat_(mat) {}

Image Image::fromScreenshot() {
    adb::takeScreenshot();
    cv::Mat image = cv::imread("screenshot.png");
    return Image(std::move(image));
}

Image Image::fromBitmap() {
    return Image(cv::Mat());
}

void Image::saveToBitmap(unsigned nonogram_width, unsigned nonogram_height) {
    const int width = mat_.cols;
    const int height = mat_.rows;
    const double cell_width = (double)width / nonogram_width;
    const double cell_height = (double)height / nonogram_height;

    cv::Mat debug = mat_.clone();
    // make gray copy to be able to skip bg cells
    cv::Mat mask = getMask().reduceNoise().mat_;
    // fill result bitmap
    cv::Mat bitmap(nonogram_height, nonogram_width, CV_8UC3, cv::Scalar(255, 255, 255));
    double x = cell_width / 2;
    double y = cell_height / 2;
    for (int row = 0; row < nonogram_height; row++) {
        x = cell_width / 2;
        for (int col = 0; col < nonogram_width; col++, x += cell_width) {
            cv::Point point{(int)x, (int)y};
            // skip white pixels
            if (mask.at<uchar>(point)) {
                cv::drawMarker(debug, point, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 10, 1);
                continue;
            }
            cv::drawMarker(debug, point, cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 10, 1);

            // TODO: naively picking color from the center may be inaccurate
            bitmap.at<cv::Vec3b>(row, col) = mat_.at<cv::Vec3b>(point);
        }
        y += cell_height;
    }

    // for debugging
    cv::imwrite("debug.png", debug);
    cv::imwrite("mask.png", mask);
    // saving
    cv::imwrite("bitmap.bmp", bitmap);
}

Image Image::cropAnswer() {
    // init sizes
    const int width = mat_.cols;
    const int height = mat_.rows;
    // crop to canvas
    cv::Mat mask = getMask().reduceNoise().mat_;
    cv::Rect bounding_box_canvas = cv::boundingRect(mask);
    // shrink it further to remove remaining pixel noise on borders
    bounding_box_canvas.x += 2;
    bounding_box_canvas.y += 2;
    bounding_box_canvas.width -= 4;
    bounding_box_canvas.height -= 4;
    cv::Mat canvas = mat_(bounding_box_canvas);
    mask = mask(bounding_box_canvas);
    // crop to answer picture
    // TODO: may actually crop wrong when nonogram has border cells colored with color close to bg
    cv::bitwise_not(mask, mask);
    cv::Rect bounding_box_picture = cv::boundingRect(mask);
    cv::Mat picture = canvas(bounding_box_picture);

    return Image(std::move(picture));
}

Image Image::getMask(bool is_inverted) {
    // create mask on grayscale image
    cv::Mat mat_gray;
    cv::cvtColor(mat_, mat_gray, cv::COLOR_BGR2GRAY);
    // create mask itself
    static constexpr int kMaxWhiteValue = 240;
    cv::Mat mask;
    int threshold_type = (is_inverted ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY);
    cv::threshold(mat_gray, mask, kMaxWhiteValue, 255, threshold_type);
    return Image(std::move(mask));
}

Image Image::reduceNoise() {
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mat_, mat_, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(mat_, mat_, cv::MORPH_OPEN, kernel);
    return *this;
}
