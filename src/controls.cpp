#include "controls.h"

#include <cstdlib>
#include <fstream>
#include <format>
#include <thread>

#include "asio.hpp"
#include "subprocess.h"

#define STR_IMPL_(x) #x
#define STR(x) STR_IMPL_(x)

using asio::ip::tcp;

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

// helper functions
namespace {
    void pushServer() {
        int result = -1;
        result = std::system("adb push ../third_party/scrcpy-server-v3.3.4 /data/local/tmp/scrcpy-server-manual.jar");
        assert(result == 0 && "adb push");
        result = std::system("adb forward tcp:" STR(SCRCPY_CLIENT_PORT) " localabstract:scrcpy");
        assert(result == 0 && "adb forward");
    }

    int runServer(subprocess_s* process) {
        const char* cmd[] = {
            "adb", "shell", "CLASSPATH=/data/local/tmp/scrcpy-server-manual.jar",
            "app_process", "/", "com.genymobile.scrcpy.Server 3.3.4",
            "tunnel_forward=true", "audio=false", "video=false", "cleanup=false",
            "send_device_meta=false", "send_frame_meta=false", "send_dummy_byte=true",
            NULL
        };
        int result = subprocess_create(
            cmd,
            subprocess_option_inherit_environment | subprocess_option_search_user_path | subprocess_option_enable_async,
            process
        );
        assert(result == 0 && "supbrocess create");
        return result;
    }

    int joinSubprocess(subprocess_s* process) {
        int ret = -1;
        int result =  subprocess_join(process, &ret);
        assert(ret == 0 && "subprocess return code");
        assert(result == 0 && "subprocess_join");
        return result;
    }

    int destroySubprocess(subprocess_s* process) {
        int result = subprocess_destroy(process);
        assert(result == 0 && "subprocess destroy");
        return result;
    }

    void write16(uint8_t* buf, uint16_t value) {
        buf[0] = value >> 8;
        buf[1] = value;
    }
}   // namespace

// internal class
class ControlSessionInternal {
public:
    ControlSessionInternal(uint16_t screen_width, uint16_t screen_height) : socket_(io_context_) {
        // initialize screen sizes
        screen_size_.width = screen_width;
        screen_size_.height = screen_height;
        screen_size_.w[0] = screen_width >> 8;
        screen_size_.w[1] = screen_width;
        screen_size_.h[0] = screen_height >> 8;
        screen_size_.h[1] = screen_height;
        // initialize server
        pushServer();
        runServer(&server_process_);
        try {
            tcp::resolver resolver(io_context_);
            auto endpoints = resolver.resolve("127.0.0.1", STR(SCRCPY_CLIENT_PORT));
            // connect to server
            while (true) {
                asio::connect(socket_, endpoints);
                // trying to read the dummy byte sent when server is ready
                std::error_code ec;
                std::array<char, 1> buffer;
                asio::read(socket_, asio::buffer(buffer), ec);
                if (!ec) {
                    break;
                }
                std::this_thread::sleep_for(100ms);
            }
            std::println("connected to scrcpy sever.");
        } catch (std::exception& e) {
            std::println("scrcpy server connection error: {}", e.what());
        }
    }

    ~ControlSessionInternal() {
        // close server connection
        socket_.shutdown(asio::socket_base::shutdown_send);
        socket_.close();
        // join and destroy server subprocess
        joinSubprocess(&server_process_);
        destroySubprocess(&server_process_);
    }

public:
    void tap(uint16_t x, uint16_t y, std::chrono::milliseconds duration) {
        // buf is serialized data that is sent to the server
        // https://github.com/Genymobile/scrcpy/blob/master/app/tests/test_control_msg_serialize.c
        uint8_t buf[] = {
            0x02, // SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT
            0x00, // AKEY_EVENT_ACTION_DOWN
            0x12, 0x34, 0x56, 0x78, 0x87, 0x65, 0x43, 0x21, // pointer id
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // touch position: x[12, 13] y[16, 17]
            screen_size_.w[0], screen_size_.w[1], screen_size_.h[0], screen_size_.h[1], // screen size
            0xff, 0xff, // pressure
            0x00, 0x00, 0x00, 0x01, // AMOTION_EVENT_BUTTON_PRIMARY (action button)
            0x00, 0x00, 0x00, 0x01, // AMOTION_EVENT_BUTTON_PRIMARY (buttons)
        };
        // write x and y positions to buf
        write16(buf + 12, x);
        write16(buf + 16, y);
        // send touch down event
        asio::write(socket_, asio::buffer(buf));
        // small delay
        std::this_thread::sleep_for(duration);
        // send touch up event
        buf[1] = 0x01; // AKEY_EVENT_ACTION_UP
        asio::write(socket_, asio::buffer(buf));
    }

private:
    struct ScreenSize {
        uint16_t width;
        uint16_t height;
        // width and height in bytes
        uint8_t w[2];
        uint8_t h[2];
    };

private:
    ScreenSize screen_size_;
    // server subprocess data
    subprocess_s server_process_;
    // client data
    asio::io_context io_context_;
    tcp::socket socket_;
};


ControlSession::ControlSession(uint16_t screen_width, uint16_t screen_height)
    : internal_(std::make_unique<ControlSessionInternal>(screen_width, screen_height)) {}

ControlSession::~ControlSession() {}

void ControlSession::stop() { internal_.reset(); }

void ControlSession::tap(uint16_t x, uint16_t y, std::chrono::milliseconds duration) {
    if (!internal_)
        return;

    internal_->tap(x, y, duration);
}
