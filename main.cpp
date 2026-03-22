#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <hidsdi.h>
#include <hidusage.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
#include <string>

#include "pxn_wheel.h"
#include "public.h"
#include "vjoyinterface.h"
#include "ffb_reader.h"

static inline double normSigned(LONG v) {
    return std::max(-1.0, std::min(1.0, static_cast<double>(v) / 32767.0));
}

static inline double normUnsigned01(LONG v) {
    return std::max(0.0, std::min(1.0, (static_cast<double>(v) + 32768.0) / 65535.0));
}

int main() {
    PxnWheelReader reader;
    if (!reader.init(L"PXN")) {
        return 1;
    }

    auto to_utf8 = [](const std::wstring& ws){
        if (ws.empty()) return std::string();
        int size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0) return std::string();
        std::string out(static_cast<size_t>(size - 1), '\0');
        WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, out.data(), size, nullptr, nullptr);
        return out;
    };

    if (!vJoyEnabled()) {
        return 1;
    }

    if (!AcquireVJD(1)) {
        return 1;
    }

    // Initialize FFB reader with auto-detected device
    FFBReader ffb_reader;
    UINT ffb_device = FFBReader::findOwnedDevice();
    if (ffb_device == 0 || !ffb_reader.init(ffb_device)) {
        //std::cerr << "[FFB] Failed to initialize FFB reader\n";
    }

    using clock = std::chrono::steady_clock;
    auto next = clock::now();

    while (true) {
        DIJOYSTATE2 js{};
        if (!reader.poll(js)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        double steering = (normSigned(js.lX) + 1.0) * 0.5;
        double clutch   = std::max(0.0, std::min(1.0, normUnsigned01(js.rglSlider[0]) * 1.2));
        double brake    = std::max(0.0, std::min(1.0, normUnsigned01(js.lRz) * 1.2));
        double gas      = std::max(0.0, std::min(1.0, normUnsigned01(js.lZ) * 1.2));

        LONG vjoy_steer  = static_cast<LONG>(steering * 32767);
        LONG vjoy_gas    = static_cast<LONG>(gas * 32767);
        LONG vjoy_brake  = static_cast<LONG>(brake * 32767);
        LONG vjoy_clutch = static_cast<LONG>(clutch * 32767);

        SetAxis(vjoy_steer, 1, HID_USAGE_GENERIC_X);
        SetAxis(vjoy_gas, 1, HID_USAGE_GENERIC_Y);
        SetAxis(vjoy_brake, 1, HID_USAGE_GENERIC_Z);
        SetAxis(vjoy_clutch, 1, HID_USAGE_GENERIC_RX);

        for (int i = 0; i < 128; ++i) {
            int vjoy_button = i + 1;
            
            if (i == 9) vjoy_button = 22;
            else if (i == 10) vjoy_button = 21;
            else if (i == 11) vjoy_button = 24;
            else if (i == 12) vjoy_button = 23;
            else if (i == 20) vjoy_button = 10;
            else if (i == 21) vjoy_button = 11;
            else if (i == 22) vjoy_button = 13;
            else if (i == 23) vjoy_button = 12;
            
            SetBtn((js.rgbButtons[i] & 0x80) ? TRUE : FALSE, 1, vjoy_button);
        }

        // Poll FFB effects
        FFBEffect ffb;
        if (ffb_reader.poll(ffb)) {
            // if (ffb.direction != 0) {
                std::cout << "[FFB] Type=" << FFBReader::effectTypeName(ffb.type)
                          << " Magnitude=" << ffb.magnitude
                          << " Direction=" << ffb.direction
                          << " Offset=" << ffb.offset << std::endl;
            // }
        }

        next += std::chrono::milliseconds(5);
        std::this_thread::sleep_until(next);
    }

    ffb_reader.close();
    RelinquishVJD(1);
    return 0;
}
