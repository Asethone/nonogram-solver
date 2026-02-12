#include <print>
#include <cxxopts.hpp>

#include "controls.h"
#include "screen_handler.h"

// captures the answer to the nonogram and saves it into generated png
void captureAnswer(bool is_colored) {
    adb::takeScreenshot();
    ScreenHandler::readAnswer();
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("solver", "Solve nonogram");
    options.add_options()
        ("help", "Print help")
        ("width", "Width of the nonogram", cxxopts::value<unsigned>())
        ("height", "Height of the nonogram", cxxopts::value<unsigned>())
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
    unsigned canvas_width = args["width"].as<unsigned>();
    unsigned canvas_height = args["height"].as<unsigned>();
    // colored
    bool is_colored = args["colored"].as<bool>();

    // check if device is connected
    if (!adb::checkDevice()) {
        std::println("Error: please connect your device via USB.");
        return 0;
    }

    if (is_capture_mode) {
        captureAnswer(is_colored);
    } else if (is_paint_mode) {
        // TODO
    } else {
        std::println("Error: specify only one mode option");
        return 0;
    }

    return 0;
}
