#include <print>
#include <cxxopts.hpp>

#include "controls.h"
#include "screen.h"

/* *
 * TODO:
 * - fill colored nonograms
 * - handle custom background colors
 * ? recognize paper pattern instead of noise reduction
 * */

int main(int argc, char* argv[]) {
    cxxopts::Options options("solver", "Solve nonogram");
    options.add_options()
        ("help", "Print help")
        ("width", "Width of the nonogram", cxxopts::value<int>())
        ("height", "Height of the nonogram", cxxopts::value<int>())
        ("c,capture", "Capture mode", cxxopts::value<bool>())
        ("p,paint", "Paint mode", cxxopts::value<bool>())
        ("colored", "Colored nonogram (default black and white)", cxxopts::value<bool>());

    options.parse_positional({"width", "height"});
    auto args = options.parse(argc, argv);

    // help
    if (args.count("help")) {
        std::println("{}", options.help());
        return 0;
    }

    // mode
    bool is_capture_mode = args["capture"].as<bool>();
    bool is_paint_mode = args["paint"].as<bool>();

    if (!is_capture_mode && !is_paint_mode) {
        std::println("Error: one mode option should be specified");
        return 0;
    }
    // width and height
    int nonogram_width = args["width"].as<int>();
    int nonogram_height = args["height"].as<int>();
    // colored
    bool is_colored = args["colored"].as<bool>();

    // check if device is connected
    // if (!adb::checkDevice()) {
    //     std::println("Error: please connect your device via USB.");
    //     return 0;
    // }

    Screen screen;
    if (is_capture_mode) {
        screen.captureAnswer(nonogram_width, nonogram_height, is_colored);
    } else if (is_paint_mode) {
        screen.paint(nonogram_width, nonogram_height, is_colored);
    } else {
        std::println("Error: specify only one mode option");
        return 0;
    }

    return 0;
}
