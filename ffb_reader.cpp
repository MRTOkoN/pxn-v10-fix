#include "ffb_reader.h"
#include "vjoyinterface.h"
#include <iostream>

FFBReader::FFBReader() {}
FFBReader::~FFBReader() { close(); }

UINT FFBReader::findOwnedDevice() {
    std::cout << "[FFB] Scanning for owned vJoy devices..." << std::endl;
    
    // Check devices 1-16
    for (UINT id = 1; id <= 16; ++id) {
        VjdStat status = GetVJDStatus(id);
        if (status == VJD_STAT_OWN) {
            std::cout << "[FFB] Found owned device: " << id << std::endl;
            return id;
        }
    }
    
    std::cerr << "[FFB] No owned vJoy device found" << std::endl;
    return 0;
}

bool FFBReader::init(UINT device_id) {
    device_id_ = device_id;
    
    std::cout << "[FFB] Initializing for vJoy Device ID: " << device_id_ << std::endl;
    
    // Basic initialization - check if device is available
    VjdStat status = GetVJDStatus(device_id_);
    
    std::cout << "[FFB] Device " << device_id_ << " status: ";
    switch (status) {
        case VJD_STAT_OWN:
            std::cout << "Owned by this feeder" << std::endl;
            break;
        case VJD_STAT_FREE:
            std::cout << "Free (not owned)" << std::endl;
            break;
        case VJD_STAT_BUSY:
            std::cout << "Owned by another feeder" << std::endl;
            break;
        case VJD_STAT_MISS:
            std::cout << "Missing (not installed)" << std::endl;
            std::cerr << "[FFB] ERROR: Device " << device_id_ << " not installed\n";
            return false;
        default:
            std::cout << "Unknown status (" << status << ")" << std::endl;
            return false;
    }
    
    std::cout << "[FFB] Monitor initialized for device " << device_id_ << std::endl;
    ffb_handle_ = reinterpret_cast<PVOID>(1);  // Mark as initialized
    return true;
}

void FFBReader::close() {
    if (ffb_handle_) {
        // vJoy doesn't require explicit cleanup, but we null it out
        ffb_handle_ = nullptr;
    }
}

bool FFBReader::poll(FFBEffect& out) {
    std::memset(&out, 0, sizeof(FFBEffect));
    return false;
}

std::string FFBReader::effectTypeName(DWORD type) {
    switch (type) {
        case 0:  return "None";
        case 1:  return "Generic";
        case 2:  return "Constant";
        case 3:  return "Ramp";
        case 4:  return "Periodic";
        case 5:  return "Spring";
        case 6:  return "Damper";
        case 7:  return "Inertia";
        case 8:  return "Friction";
        default: return "Unknown(" + std::to_string(type) + ")";
    }
}
