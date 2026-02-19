#include "image.h"

#include "controls.h"

Image::Image() {}

Image::Image(const cv::Mat& mat) : mat_(mat) {}

Image::Image(const cv::Mat& mat, const cv::Rect& rect) : mat_(mat), rect_(rect) {}

Image Image::fromScreenshot() {
    cv::Mat image = cv::imread("screenshot.png");
    return Image(std::move(image));
}

Image Image::fromBitmap(bool is_colored) {
    cv::Mat image = cv::imread("bitmap.bmp", (is_colored ? cv::IMREAD_COLOR_BGR : cv::IMREAD_GRAYSCALE));
    return Image(std::move(image));
}

void Image::saveToBitmap(int nonogram_width, int nonogram_height) {
    const int width = mat_.cols;
    const int height = mat_.rows;
    const double cell_width = (double)width / nonogram_width;
    const double cell_height = (double)height / nonogram_height;

    cv::Mat debug = mat_.clone();
    // make mask to be able to skip bg cells
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

    cv::Rect getLongestHorizontalLine(cv::Mat mat) {
        cv::Rect res{0, 0, 0, 1};
        auto update = [&](int len, int x_end, int y) {
            if (len > res.width) {
                res.width = len;
                res.x = x_end - len;
                res.y = y;
            }
        };
        for (int row = 0; row < mat.rows; row++) {
            int curr_len = 0;
            for (int col = 0; col < mat.cols; col++) {
                if (mat.at<uchar>(cv::Point(col, row))) {
                    curr_len++;
                } else {
                    update(curr_len, col, row);
                    curr_len = 0;
                }
            }
            update(curr_len, mat.cols, row);
        }
        return res;
    }

    cv::Rect getLongestVerticalLine(cv::Mat mat) {
        cv::Rect res{0, 0, 1, 0};
        auto update = [&](int len, int x, int y_end) {
            if (len > res.height) {
                res.height = len;
                res.x = x;
                res.y = y_end - len;
            }
        };
        for (int col = 0; col < mat.cols; col++) {
            int curr_len = 0;
            for (int row = 0; row < mat.rows; row++) {
                if (mat.at<uchar>(cv::Point(col, row))) {
                    curr_len++;
                } else {
                    update(curr_len, col, row);
                    curr_len = 0;
                }
            }
            update(curr_len, col, mat.rows);
        }
        return res;
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
    constexpr int kDarkestPaperPixelGrayValue = 230;
    Image mask_image = getMask(kDarkestPaperPixelGrayValue);
    cv::Mat mask = mask_image.mat_;
    // reduce noise on white regions
    // cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 5));
    // cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // extract white horizontal lines that wrap canvas
    /* TODO: in case when the device is in horizontal orientation or
     *  the nonogram is too tall it sticks to the top-bottom borders,
     *  we should go for vertical lines instead
     */
    cv::Mat horizontal = mask.clone();
    const int horizontal_size = mat_.cols;
    cv::Mat horizontal_structure = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(horizontal_size, 1));
    cv::erode(horizontal, horizontal, horizontal_structure);

    // cv::imwrite("mask.png", mask);
    // cv::imwrite("debug.png", horizontal);

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

Image Image::extractGrid(cv::Scalar& bg_color, int width, int height) {
    constexpr int kDarkestPaperPixelGrayValue = 220;
    cv::Mat mask = getMask(kDarkestPaperPixelGrayValue).mat_;

    // extract preview rect
    cv::Mat horizontal = mask.clone();
    cv::Mat vertical = mask.clone();
    // let's assume the preview rect takes up slightly bigger size than one grid cell (minimum)
    // TODO: that might be pretty rough, better find more reliable solution (detect preview rect black borders?)
    const int horizontal_size = (int)(mat_.cols / width);
    const int vertical_size = (int)(mat_.rows / height);
    cv::Mat horizontal_structure = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(horizontal_size, 1));
    cv::erode(horizontal, horizontal, horizontal_structure);
    cv::Mat vertical_structure = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, vertical_size));
    cv::erode(vertical, vertical, vertical_structure);
    // merge horizontal & vertical lines into one mask
    cv::Mat mask_corner_image;
    cv::bitwise_or(horizontal, vertical, mask_corner_image);
    // instead of taking bounding rect, get longest lines that would correspond to the preview position
    cv::Rect horizontal_longest = getLongestHorizontalLine(mask_corner_image);
    cv::Rect vertical_longest = getLongestVerticalLine(mask_corner_image);
    cv::Rect bounding_box_preview(
        horizontal_longest.x,
        vertical_longest.y,
        horizontal_longest.width,
        vertical_longest.height
    );

    // debugging
    // cv::Mat debug = mat_.clone();
    // cv::rectangle(debug, horizontal_longest, cv::Scalar(0, 255, 0));
    // cv::rectangle(debug, vertical_longest, cv::Scalar(0, 0, 255));
    // cv::imwrite("debug.png", debug);
    // cv::imwrite("mask.png", mask);
    // cv::imwrite("mask_corner_image.png", mask_corner_image);

    if (bounding_box_preview.area() == 0) {
        cv::imwrite("mask.png", mask);
        cv::imwrite("mask_corner_image.png", mask_corner_image);
        throw std::runtime_error("error: unable to extract preview rect");
    }

    // by default the preview is filled with background color
    // now we can obtain it taking the center pixel of the preview rect
    cv::Point preview_center(
        bounding_box_preview.x + bounding_box_preview.width / 2,
        bounding_box_preview.y + bounding_box_preview.height / 2
    );
    bg_color = mat_.at<cv::Vec3b>(preview_center);
    // for debugging
    // cv::Mat debug = mat_.clone();
    // cv::drawMarker(debug, preview_center, cv::Scalar(0, 255, 0), cv::MARKER_CROSS);
    // cv::imwrite("nonogram.png", debug);

    // finally, extract the grid
    int grid_x = 2 * bounding_box_preview.x + bounding_box_preview.width;
    int grid_y = 2 * bounding_box_preview.y + bounding_box_preview.height;
    int grid_width = mat_.cols - grid_x - bounding_box_preview.x;
    int grid_height = mat_.rows - grid_y - bounding_box_preview.y;
    cv::Rect bounding_box_grid(grid_x, grid_y, grid_width, grid_height);

    if (bounding_box_grid.area() == 0) {
        cv::imwrite("preview.png", mat_(bounding_box_preview));
        throw std::runtime_error("error: unable to extract grid");
    }

    return Image(mat_(bounding_box_grid), toAbsoluteRect(rect_, bounding_box_grid));
}
