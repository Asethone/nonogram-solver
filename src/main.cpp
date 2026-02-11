#include <print>
#include <cxxopts.hpp>

int main(int argc, char* argv[]) {
    cxxopts::Options options("solver", "Solve nonogram");
    options.add_options()
        ("width", "Width of the nonogram", cxxopts::value<unsigned>())
        ("height", "Height of the nonogram", cxxopts::value<unsigned>())
        ("c,capture", "Capture mode", cxxopts::value<bool>())
        ("p,paint", "Paint mode", cxxopts::value<bool>());

    options.parse_positional({"width", "height"});
    auto args = options.parse(argc, argv);

    bool is_capture_mode = args["capture"].as<bool>();
    bool is_paint_mode = args["paint"].as<bool>();

    if (!is_capture_mode && !is_paint_mode) {
        std::println("One mode option should be specified");
        return 0;
    }

    unsigned canvas_width = args["width"].as<unsigned>();
    unsigned canvas_height = args["height"].as<unsigned>();

    if (is_capture_mode) {

    } else if (is_paint_mode) {

    } else {
        std::println("Specify only one mode option");
        return 0;
    }

    return 0;
}
