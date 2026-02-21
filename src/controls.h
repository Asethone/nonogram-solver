#pragma once

#include <chrono>
#include <memory>
#include <cstdint>

#define SCRCPY_CLIENT_PORT 1234

using namespace std::literals;
using std::uint16_t;
using std::uint8_t;

namespace adb {
    // returns true if device is connected, false otherwise
    bool checkDevice();

    // take a screenshot
    void takeScreenshot();

    /*
     * NOTE:
     * The following functions are terribly slow due to the overhead of starting
     * a new Java process on Android device for every single call.
     * For controlling device better use ControlSession class.
     */

    // do a single tap
    void tap(unsigned x, unsigned y);
    // do a swipe from one point to another
    void swipe(unsigned x1, unsigned y1, unsigned x2, unsigned y2, std::chrono::milliseconds duration);

}   // namespace adb

class ControlSessionInternal;
class ControlSession {
public:
    ControlSession(uint16_t screen_width, uint16_t screen_height);
    // disabled copy and move operations
    ControlSession(const ControlSession&) = delete;
    ControlSession& operator=(const ControlSession&) = delete;
    ControlSession(ControlSession&&) = delete;
    ControlSession&& operator=(ControlSession&&) = delete;
    ~ControlSession();

    // end session and release resources (which is made automatically on destruction)
    void stop();

public:
    // do a single tap
    void tap(uint16_t x, uint16_t y, std::chrono::milliseconds duration = 5ms);

private:
    std::unique_ptr<ControlSessionInternal> internal_;
};
