cmake_minimum_required(VERSION 3.10)
project(openups CXX C)

set(SOURCES 
src/main.cpp
src/lib/usbhid.cpp
src/lib/HIDOpenUPS.cpp
src/lib/HIDOpenUPS2.cpp
src/lib/HIDNUCUPS.cpp
src/lib/HIDDCDCUSB.cpp
src/lib/HIDInterface.cpp
src/utils/util.cpp
src/utils/HArray.cpp 
)

option(_DEBUG "Debug messages" ON)
if (_DEBUG)
  add_definitions(-D_DEBUG)
endif()

add_executable(openups ${SOURCES})


target_include_directories(openups PUBLIC src/utils/ src/lib/)
target_link_libraries(openups
#pthread
usb
)

install(
  TARGETS openups
  DESTINATION bin
  )

# Snap Build
# snapcraft clean
# snapcraft
# snapcraft upload --release=beta openups_0.4_amd64.snap