#include "screen.h"

#include <algorithm>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <thread>

#include "controls.h"

Screen::Screen() {
    update();
}

void Screen::update() {
    adb::takeScreenshot();
    screen_image_ = Image::fromScreenshot();
}

void Screen::captureAnswer(int width, int height, bool is_colored) {
    screen_image_.extractAnswer().saveToBitmap(width, height, is_colored);
}

namespace {
    int findClosestColor(const std::vector<cv::Vec3b>& palette, const cv::Vec3b& target) {
        auto distSq = [&](const cv::Vec3b& color) {
            int dr = (int)color[0] - target[0];
            int dg = (int)color[1] - target[1];
            int db = (int)color[2] - target[2];
            return dr*dr + dg*dg + db*db;
        };

        auto comp = [&](const cv::Vec3b& a, const cv::Vec3b& b) {
            return distSq(a) < distSq(b);
        };

        return std::distance(palette.begin(), std::min_element(palette.begin(), palette.end(), comp));
    }
}

void Screen::paint(int width, int height, bool is_colored) {
    // parse nonogram
    Image nonogram = screen_image_.extractNonogram();
    cv::imwrite("nonogram.png", nonogram.mat_);
    // parse grid and background color
    cv::Vec3b bg_color;
    Image grid = nonogram.extractGrid(bg_color, width, height);
    std::println("background color is {}, {}, {}", bg_color[0], bg_color[1], bg_color[2]);
    // calculate cell sizes and x, y position
    const double cell_width = (double)grid.mat_.cols / width;
    const double cell_height = (double)grid.mat_.rows / height;
    double x = grid.rect_.x + cell_width / 2;
    double y = grid.rect_.y + cell_height / 2;
    if (is_colored) {
        cv::Mat answer = Image::fromBitmap(is_colored).mat_;
        // for debugging
        cv::Mat debug = screen_image_.mat_.clone();
        // parse color palette
        std::vector<cv::Vec3b> palette_colors;
        std::vector<cv::Point> color_coords;
        Image palette = screen_image_.extractPalette(palette_colors, color_coords, nonogram.rect_);
        const int color_count = palette_colors.size();
        palette_colors.push_back(bg_color);
        // start painting
        int last_picked_color = -1;
        ControlSession ctrl(screen_image_.mat_.cols, screen_image_.mat_.rows);
        for (int row = 0; row < height; row++) {
            x = grid.rect_.x + cell_width / 2;
            for (int col = 0; col < width; col++, x += cell_width) {
                cv::Vec3b answer_cell_color = answer.at<cv::Vec3b>(cv::Point(col, row));
                int i_color = findClosestColor(palette_colors, answer_cell_color);
                // the last color of the vector is bg color, skip it
                if (i_color == color_count)
                    continue;
                // tap the color first if we need to change it
                if (i_color != last_picked_color) {
                    ctrl.tap(color_coords[i_color].x, color_coords[i_color].y);
                    // after tapping the color the application need some time to apply it
                    std::this_thread::sleep_for(80ms);
                }
                last_picked_color = i_color;
                // then tap the cell
                ctrl.tap(x, y);

                // for debugging
                cv::rectangle(debug, { (int)(x - cell_width / 2), (int)(y - cell_height / 2), (int)cell_width, (int)cell_height }, palette_colors[i_color], cv::FILLED);
            }
            y += cell_height;
        }
        cv::imwrite("debug.png", debug);
    } else {
        cv::Mat answer = Image::fromBitmap(is_colored).mat_;
        // for debugging
        cv::Mat debug = screen_image_.mat_.clone();
        // start painting
        ControlSession ctrl(screen_image_.mat_.cols, screen_image_.mat_.rows);
        for (int row = 0; row < height; row++) {
            x = grid.rect_.x + cell_width / 2;
            for (int col = 0; col < width; col++, x += cell_width) {
                if (answer.at<uchar>(cv::Point(col, row)) < 230) {
                    ctrl.tap(x, y);
                    cv::rectangle(debug, { (int)(x - cell_width / 2), (int)(y - cell_height / 2), (int)cell_width, (int)cell_height }, cv::Scalar(0, 0, 0), cv::FILLED);
                }
            }
            y += cell_height;
        }
        cv::imwrite("debug.png", debug);
    }
}
