cmake_minimum_required(VERSION 3.15)
project(pxn_virtual_wheel)

set(CMAKE_CXX_STANDARD 17)

# Path to the official vJoy SDK headers (contains public.h and vjoyinterface.h)
set(VJOY_SDK_INC "C:/Users/del-t/Downloads/vJoy-2.1.9.1/vJoy-2.1.9.1/SDK/inc" CACHE PATH "vJoy SDK include path")

add_executable(pxn_virtual_wheel
    main.cpp
    pxn_wheel.cpp
    ffb_reader.cpp
)

target_include_directories(pxn_virtual_wheel PRIVATE
    ${CMAKE_SOURCE_DIR}/vjoy
    ${VJOY_SDK_INC}
)

target_link_libraries(pxn_virtual_wheel
    setupapi
    hid
    dinput8
    dxguid
    ${CMAKE_SOURCE_DIR}/vjoy/vJoyInterface.lib
)
