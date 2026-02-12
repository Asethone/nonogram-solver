#pragma once

#include <cstdlib>
#include <fstream>
#include <format>
#include <chrono>

using namespace std::literals;

namespace adb {
    // returns true if device is connected, false otherwise
    bool checkDevice();

    // take a screenshot
    void takeScreenshot();

    // do a single tap
    void tap(unsigned x, unsigned y);

    // do a swipe from one point to another
    void swipe(unsigned x1, unsigned y1, unsigned x2, unsigned y2, std::chrono::milliseconds duration);

}   // namespace adb
