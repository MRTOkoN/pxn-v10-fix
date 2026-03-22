#pragma once

#include <windows.h>
#include <string>
#include <cstring>

// Simple FFB effect structure
struct FFBEffect {
    DWORD type;           // Effect type (constant, periodic, ramp, etc.)
    DWORD direction;      // Direction (0-360 degrees)
    LONG magnitude;       // Magnitude (-10000 to 10000)
    LONG offset;          // Offset (-10000 to 10000)
    LONG phase;           // Phase (0-36000)
    DWORD period;         // Period (ms)
    LONG envelope_attack; // Attack level
    LONG envelope_fade;   // Fade level
    DWORD envelope_attack_time; // Attack time (ms)
    DWORD envelope_fade_time;   // Fade time (ms)
};

class FFBReader {
public:
    FFBReader();
    ~FFBReader();

    // Initialize FFB reading for vJoy device
    bool init(UINT device_id = 1);
    void close();
    
    // Auto-detect first owned vJoy device (0 if none found)
    static UINT findOwnedDevice();

    // Poll for FFB effects from the device
    bool poll(FFBEffect& out);

    // Get human-readable effect type name
    static std::string effectTypeName(DWORD type);
    
    // Get current device ID
    UINT deviceId() const { return device_id_; }

private:
    UINT device_id_ = 1;
    PVOID ffb_handle_ = nullptr;
};
