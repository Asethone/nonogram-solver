#pragma once

#include <cstdlib>
#include <fstream>
#include <format>
#include <chrono>

using namespace std::literals;

namespace adb {
    // returns true if device is connected, false otherwise
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

    // take a screenshot
    void takeScreenshot() {
        std::system("adb exec-out screencap -p > screenshot.png");
    }

    // do a single tap
    void tap(unsigned x, unsigned y) {
        std::string cmd = std::format("adb shell input tap {} {}", x, y);
        std::system(cmd.c_str());
    }

    // do a swipe from one point to another
    void swipe(unsigned x1, unsigned y1, unsigned x2, unsigned y2, std::chrono::milliseconds duration) {
        std::string cmd = std::format("adb shell input swipe {} {} {} {} {}", x1, y1, x2, y2, duration.count());
        std::system(cmd.c_str());
    }
}   // namespace adb
