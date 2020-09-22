cmake_minimum_required(VERSION 3.10)
project(openups2)

set(SOURCES 
main.cpp
lib/HIDOpenUPS2.cpp
lib/HIDGraphics.cpp
lib/HIDInterface.cpp
lib/HIDDevice.cpp
lib/HIDDevices.cpp
lib/HIDDescriptor.cpp
lib/DeviceDetector.cpp
lib/OpenUPSConnector.cpp
utils/stdafx.cpp
utils/HArray.cpp 
utils/HList.cpp 
utils/HLock.cpp  
utils/HThread.cpp 
utils/TxFile.cpp)

option(_DEBUG "Debug messages" ON)
if (_DEBUG)
  add_definitions(-D_DEBUG)
endif()

add_executable(openups2 ${SOURCES})

target_include_directories(openups2 PUBLIC utils/ lib/)
target_link_libraries(openups2
pthread
)
