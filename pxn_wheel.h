#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dinput.h>
#include <string>
#include <vector>

class PxnWheelReader {
public:
    PxnWheelReader();
    ~PxnWheelReader();

    bool init(const std::wstring& nameContains = L"PXN");
    void close();

    bool poll(DIJOYSTATE2& out);

    std::wstring deviceName() const { return device_name_; }
    const std::vector<std::wstring>& axesNames() const { return axes_names_; }
    int buttonCount() const { return button_count_; }
    int povCount() const { return pov_count_; }

private:
    static BOOL CALLBACK EnumJoysticksCallback(LPCDIDEVICEINSTANCEW pdidInstance, VOID* pContext);
    static BOOL CALLBACK EnumObjectsCallback(LPCDIDEVICEOBJECTINSTANCEW pdidoi, VOID* pContext);

    bool selectDevice(const GUID& instanceGuid);
    bool configureDevice();

private:
    IDirectInput8W* di_ = nullptr;
    IDirectInputDevice8W* dev_ = nullptr;
    std::wstring wanted_substr_;
    std::wstring device_name_;
    GUID chosen_guid_{};
    bool device_chosen_ = false;

    std::vector<std::wstring> axes_names_;
    int button_count_ = 0;
    int pov_count_ = 0;
};
