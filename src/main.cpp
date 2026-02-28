#include <iostream>
#include <cxxopts.hpp>

#include "controls.h"
#include "screen.h"

/* *
 * TODO:
 * - handle nonograms with white borders
 * */

int main(int argc, char* argv[]) {
    cxxopts::Options options("solver", "Solve nonogram");
    options.add_options()
        ("help", "Print help")
        ("width", "Width of the nonogram", cxxopts::value<int>())
        ("height", "Height of the nonogram", cxxopts::value<int>())
        ("c,capture", "Capture mode", cxxopts::value<bool>())
        ("p,paint", "Paint mode", cxxopts::value<bool>())
        ("o,colored", "Colored nonogram (default black and white)", cxxopts::value<bool>());

    options.parse_positional({"width", "height"});
    auto args = options.parse(argc, argv);

    // help
    if (args.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // mode
    bool is_capture_mode = args["capture"].as<bool>();
    bool is_paint_mode = args["paint"].as<bool>();

    if (!is_capture_mode && !is_paint_mode) {
        std::cout << "No mode options were specified. Going with multimode." << std::endl;
        is_capture_mode = true;
        is_paint_mode = true;
    }

    bool is_multimode = (is_capture_mode && is_paint_mode);

    // width and height
    int nonogram_width = args["width"].as<int>();
    int nonogram_height = args["height"].as<int>();
    // colored
    bool is_colored = args["colored"].as<bool>();

    // check if device is connected
    if (!adb::checkDevice()) {
        std::cout << "Error: please connect your device via USB." << std::endl;
        return 1;
    }

    // run
    Screen screen;
    if (is_capture_mode) {
        screen.captureAnswer(nonogram_width, nonogram_height, is_colored);
    }
    if (is_paint_mode) {
        screen.paint(nonogram_width, nonogram_height, is_colored, is_multimode);
    }

    return 0;
}
