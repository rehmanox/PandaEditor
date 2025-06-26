cmake_minimum_required(VERSION 3.16)
project(wx-example)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify WxWidgets install location
set(wxWidgets_DIR "../wx_install/lib/cmake/wxWidgets")

# Get WxWidgets package
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base)

# Define target
add_executable(wx-example main.cpp)

# Link and include wx
target_link_libraries(wx-example PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(wx-example PRIVATE ${wxWidgets_INCLUDE_DIRS})
