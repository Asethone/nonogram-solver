#include <print>
#include <cxxopts.hpp>

#include "controls.h"

// captures the answer to the nonogram and saves it into generated png
void captureAnswer() {
    adb::swipe(200, 1800, 800, 1800, 200ms);
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("solver", "Solve nonogram");
    options.add_options()
        ("width", "Width of the nonogram", cxxopts::value<unsigned>())
        ("height", "Height of the nonogram", cxxopts::value<unsigned>())
        ("c,capture", "Capture mode", cxxopts::value<bool>())
        ("p,paint", "Paint mode", cxxopts::value<bool>());

    options.parse_positional({"width", "height"});
    auto args = options.parse(argc, argv);
    // mode
    bool is_capture_mode = args["capture"].as<bool>();
    bool is_paint_mode = args["paint"].as<bool>();

    if (!is_capture_mode && !is_paint_mode) {
        std::println("Error: one mode option should be specified");
        return 0;
    }
    // width and height
    unsigned canvas_width = args["width"].as<unsigned>();
    unsigned canvas_height = args["height"].as<unsigned>();

    // check if device is connected
    if (!adb::checkDevice()) {
        std::println("Error: please connect your device via USB.");
        return 0;
    }

    if (is_capture_mode) {
        captureAnswer();
    } else if (is_paint_mode) {
        // TODO
    } else {
        std::println("Error: specify only one mode option");
        return 0;
    }

    return 0;
}
