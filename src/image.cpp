#include "image.h"

#include "controls.h"

Image::Image() {}

Image::Image(const cv::Mat& mat) : mat_(mat) {}

Image::Image(const cv::Mat& mat, const cv::Rect& rect) : mat_(mat), rect_(rect) {}

Image Image::fromScreenshot() {
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

Image Image::getMask(int thresh, bool is_inverted) {
    // create mask on grayscale image
    cv::Mat gray;
    cv::cvtColor(mat_, gray, cv::COLOR_BGR2GRAY);
    // create mask
    cv::Mat mask;
    int threshold_type = (is_inverted ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY);
    cv::threshold(gray, mask, thresh, 255, threshold_type);
    return Image(std::move(mask));
}

Image Image::reduceNoise() {
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
    cv::morphologyEx(mat_, mat_, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(mat_, mat_, cv::MORPH_OPEN, kernel);
    return *this;
}

namespace {
    // translates child rect to the coordinate system of parent rect
    cv::Rect toAbsoluteRect(cv::Rect parent, cv::Rect child) {
        return cv::Rect(child + cv::Point(parent.x, parent.y));
    }
}

Image Image::extractAnswer() {
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
    // TODO: may actually crop wrong when nonogram has border cells colored with colors close to bg
    cv::bitwise_not(mask, mask);
    cv::Rect bounding_box_picture = cv::boundingRect(mask);
    cv::Mat picture = canvas(bounding_box_picture);

    return Image(
        std::move(picture),
        toAbsoluteRect(bounding_box_canvas, bounding_box_picture)
    );
}

Image Image::extractNonogram() {
    Image mask_image = getMask();
    cv::Mat mask = mask_image.mat_;
    // reduce noise on white regions
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // extract white horizontal lines that wrap canvas
    /* TODO: in case when the device is in horizontal orientation or
     *  the nonogram is too tall it sticks to the top-bottom borders,
     *  we should go for vertical lines instead
     */
    cv::Mat horizontal = mask.clone();
    int horizontal_size = mat_.cols;
    cv::Mat horizontal_structure = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(horizontal_size, 1));
    cv::erode(horizontal, horizontal, horizontal_structure);
    // get canvas bounding box
    cv::Rect bounding_box_canvas = cv::boundingRect(horizontal);
    if (bounding_box_canvas.area() == 0) {
        cv::imwrite("mask.png", mask);
        throw std::runtime_error("error: unable to extract canvas");
    }

    // remove white margins
    cv::Mat canvas = mat_(bounding_box_canvas);
    mask = Image(canvas).getMask(100, true).mat_;
    cv::Rect bounding_box_nonogram = cv::boundingRect(mask);

    return Image(
        std::move(canvas(bounding_box_nonogram)),
        toAbsoluteRect(bounding_box_canvas, bounding_box_nonogram)
    );
}

Image Image::extractGrid(cv::Scalar& bg_color) { return Image(); }
