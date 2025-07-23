cmake_minimum_required(VERSION 3.30)

# vcpkg toolchain 自动检测与说明（必须在project之前）
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "F:/runtime/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg target triplet")
    message(STATUS "已自动设置: -DCMAKE_TOOLCHAIN_FILE=F:/runtime/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows")
endif ()

project(cpp_shopping LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_WIN32_WINNT=0x0A00)

include_directories(source/include)

# 模块文件
file(GLOB_RECURSE MODULES_FILES
        CONFIGURE_DEPENDS
        source/src/*.ixx
        source/src/*.cppm
)

# 头文件
file(GLOB_RECURSE CURRENT_HEADERS
        source/src/*.h
        source/src/*.hpp
)

# 源文件
file(GLOB_RECURSE CURRENT_SOURCE
        source/src/*.cpp
)

# 测试文件
file(GLOB_RECURSE TEST_SOURCE
        source/tests/*.cpp
)

# Boost查找
find_package(Boost REQUIRED COMPONENTS system thread json beast asio log log_setup pfr)
message(STATUS "Found Boost version: ${Boost_VERSION}")

# RTTR库
find_package(rttr CONFIG REQUIRED)
message(STATUS "Found RTTR version: ${rttr_VERSION}")

# TinyXML2库
find_package(tinyxml2 CONFIG REQUIRED)
message(STATUS "Found TinyXML2")


# 添加测试可执行文件（可选构建）
option(BUILD_TESTS "Build test cases" ON)
if (BUILD_TESTS)
    add_executable(cpp_shopping_tests
            ${TEST_SOURCE}
            ${MODULES_FILES}
            ${CURRENT_SOURCE}
            ${CURRENT_HEADERS}
    )

    target_link_libraries(cpp_shopping_tests PRIVATE
            ${Boost_LIBRARIES}
            RTTR::Core
            tinyxml2::tinyxml2
    )

    set_target_properties(cpp_shopping_tests PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
else ()
    # 构建主程序
    add_executable(cpp_shopping source/main.cpp
            ${MODULES_FILES}
            ${CURRENT_SOURCE}
            ${CURRENT_HEADERS}
    )

    # 链接主程序
    target_link_libraries(cpp_shopping PRIVATE
            ${Boost_LIBRARIES}
            RTTR::Core
            tinyxml2::tinyxml2
    )

    # 指定输出目录
    set_target_properties(cpp_shopping PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif ()