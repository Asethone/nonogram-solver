#include "controls.h"

namespace adb {
    bool checkDevice() {
        std::system("adb devices > devices.txt");
        // read devices.txt and check if any devices are there
        std::ifstream file("devices.txt");
        if (!file.is_open())
            return false;
        std::string line;
        for (int i = 0; i < 2; i++) {
            // read two lines (we only need the second one)
            if (!std::getline(file, line)) {
                return false;
            }
        }
        return !line.empty();
    }

    void takeScreenshot() {
        std::system("adb exec-out screencap -p > screenshot.png");
    }

    void tap(unsigned x, unsigned y) {
        std::string cmd = std::format("adb shell input tap {} {}", x, y);
        std::system(cmd.c_str());
    }

    void swipe(unsigned x1, unsigned y1, unsigned x2, unsigned y2, std::chrono::milliseconds duration) {
        std::string cmd = std::format("adb shell input swipe {} {} {} {} {}", x1, y1, x2, y2, duration.count());
        std::system(cmd.c_str());
    }
}   // namespace adb
