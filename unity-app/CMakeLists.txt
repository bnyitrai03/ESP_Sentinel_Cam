# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
include($ENV{IDF_PATH}/tools/cmake/version.cmake)

# Add newly added components to one of these lines:
# 1. Add here if the component is compatible with IDF >= v4.3
set(EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/tools/unit-test-app/components" "../components")
set(TEST_COMPONENTS "button" "camera" "communication" "event" "led" "mytime" "qr" "sensors" "storage" "utilities")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(example_test_app)