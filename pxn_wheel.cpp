#include "pxn_wheel.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

static std::string to_utf8(const std::wstring& ws) {
    if (ws.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();
    std::string out(static_cast<size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, out.data(), size, nullptr, nullptr);
    return out;
}

PxnWheelReader::PxnWheelReader() {}
PxnWheelReader::~PxnWheelReader() { close(); }

BOOL CALLBACK PxnWheelReader::EnumJoysticksCallback(LPCDIDEVICEINSTANCEW pdidInstance, VOID* pContext) {
    auto* self = reinterpret_cast<PxnWheelReader*>(pContext);
    if (!self) return DIENUM_CONTINUE;

    std::wstring name = pdidInstance->tszProductName;
    if (!self->wanted_substr_.empty()) {
        if (name.find(self->wanted_substr_) == std::wstring::npos) {
            return DIENUM_CONTINUE;
        }
    }

    self->chosen_guid_ = pdidInstance->guidInstance;
    self->device_name_ = name;
    self->device_chosen_ = true;
    return DIENUM_STOP;
}

BOOL CALLBACK PxnWheelReader::EnumObjectsCallback(LPCDIDEVICEOBJECTINSTANCEW pdidoi, VOID* pContext) {
    auto* self = reinterpret_cast<PxnWheelReader*>(pContext);
    if (!self) return DIENUM_CONTINUE;

    if (pdidoi->dwType & DIDFT_AXIS) {
        self->axes_names_.push_back(pdidoi->tszName);
    }
    if (pdidoi->dwType & DIDFT_BUTTON) {
        self->button_count_++;
    }
    if (pdidoi->dwType & DIDFT_POV) {
        self->pov_count_++;
    }
    return DIENUM_CONTINUE;
}

bool PxnWheelReader::init(const std::wstring& nameContains) {
    wanted_substr_ = nameContains;

    std::cout << "[DI] Init DirectInput..." << std::endl;
    if (FAILED(DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8W, (VOID**)&di_, nullptr))) {
        std::cerr << "[DI] DirectInput8Create: FAILED" << std::endl;
        return false;
    }

    std::cout << "[DI] Search device contains: '" << to_utf8(wanted_substr_) << "'..." << std::endl;
    device_chosen_ = false;
    di_->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);

    if (!device_chosen_) {
        std::cout << "[DI] Not found by substring, try first attached..." << std::endl;
        wanted_substr_.clear();
        di_->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
    }

    if (!device_chosen_) {
        std::cerr << "[DI] No game controller found" << std::endl;
        return false;
    }

    if (!selectDevice(chosen_guid_)) return false;
    if (!configureDevice()) return false;

    return true;
}

bool PxnWheelReader::selectDevice(const GUID& instanceGuid) {
    if (FAILED(di_->CreateDevice(instanceGuid, &dev_, nullptr))) {
        std::wcerr << L"[DI] CreateDevice failed" << std::endl;
        return false;
    }
    return true;
}

bool PxnWheelReader::configureDevice() {
    if (!dev_) return false;

    std::cout << "[DI] Set data format (DIJOYSTATE2)..." << std::endl;
    if (FAILED(dev_->SetDataFormat(&c_dfDIJoystick2))) {
        std::cerr << "[DI] SetDataFormat(DIJOYSTATE2): FAILED" << std::endl;
        return false;
    }

    HWND hwnd = GetConsoleWindow();
    if (!hwnd) hwnd = GetForegroundWindow();
    HRESULT hrCoop = dev_->SetCooperativeLevel(hwnd ? hwnd : GetDesktopWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    if (FAILED(hrCoop)) {
        std::cerr << "[DI] SetCooperativeLevel: WARN (failed to set)" << std::endl;
    } else {
        std::cout << "[DI] Cooperative: NONEXCLUSIVE | BACKGROUND" << std::endl;
    }

    DIPROPRANGE diprg;
    diprg.diph.dwSize = sizeof(DIPROPRANGE);
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprg.diph.dwHow = DIPH_DEVICE;
    diprg.diph.dwObj = 0;
    diprg.lMin = -32768;
    diprg.lMax = 32767;
    if (FAILED(dev_->SetProperty(DIPROP_RANGE, &diprg.diph))) {
        std::cerr << "[DI] Axis range: WARN (driver ignored)" << std::endl;
    } else {
        std::cout << "[DI] Axis range: [-32768..32767]" << std::endl;
    }

    axes_names_.clear();
    button_count_ = 0;
    pov_count_ = 0;
    dev_->EnumObjects(EnumObjectsCallback, this, DIDFT_ALL);
    std::cout << "[DI] Axes: " << axes_names_.size()
              << ", Buttons: " << button_count_
              << ", POV: " << pov_count_ << std::endl;
    if (!axes_names_.empty()) {
        std::cout << "[DI] Axis names: ";
        for (size_t i = 0; i < axes_names_.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << to_utf8(axes_names_[i]);
        }
        std::cout << std::endl;
    }

    HRESULT hr = dev_->Acquire();
    if (FAILED(hr)) {
        std::cerr << "[DI] Acquire: FAILED" << std::endl;
        return false;
    }
    std::cout << "[DI] Device acquired. Ready." << std::endl;
    return true;
}

bool PxnWheelReader::poll(DIJOYSTATE2& out) {
    if (!dev_) return false;

    HRESULT hr = dev_->Poll();
    if (FAILED(hr)) {
        hr = dev_->Acquire();
        while (hr == DIERR_INPUTLOST) hr = dev_->Acquire();
        if (FAILED(hr)) return false;
    }

    hr = dev_->GetDeviceState(sizeof(DIJOYSTATE2), &out);
    if (FAILED(hr)) return false;
    return true;
}

void PxnWheelReader::close() {
    if (dev_) {
        dev_->Unacquire();
        dev_->Release();
        dev_ = nullptr;
    }
    if (di_) {
        di_->Release();
        di_ = nullptr;
    }
}
