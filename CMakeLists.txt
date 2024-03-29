cmake_minimum_required(VERSION 2.8)
project(AVM)
set(CMAKE_CXX_FLAGS "-std=c++17 -g -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
SET(CMAKE_BUILD_TYPE "Release")

include_directories("/usr/include/eigen3")

find_package(OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})
enable_language("CUDA")
set(CMAKE_CUDA_STANDARD 14)
set(CUDA_SEPARABLE_COMPILATION ON)
string(APPEND CMAKE_CUDA_FLAGS " -rdc=true --cudart shared")


link_directories(${OpenCV_INSTALL_PATH}/lib)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE PARSER_PATH src/*.cc)
add_library(${PROJECT_NAME} STATIC ${PARSER_PATH})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

add_executable(run_avm src/AVM.cc)
target_link_libraries(run_avm ${PROJECT_NAME})
target_link_libraries(run_avm jetson-utils)
add_executable(test src/camera-ox03cc.cpp)
target_link_libraries(test ${PROJECT_NAME})
add_executable(cuda_test src/jetson_utils_test.cc)
target_link_libraries(cuda_test ${PROJECT_NAME})
target_link_libraries(cuda_test jetson-utils)
