cmake_minimum_required(VERSION 3.14)

find_program(VCPKG_EXE vcpkg
    PATHS "$ENV{VCPKG_ROOT}" "$ENV{USERPROFILE}/vcpkg"
)
if(VCPKG_EXE)
    get_filename_component(VCPKG_ROOT "${VCPKG_EXE}" DIRECTORY)
    message(STATUS "Found vcpkg: ${VCPKG_ROOT}")
endif()

project(ApiSender VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(jsoncpp_lib STATIC
    include/jsoncpp/jsoncpp.cpp)

add_library(cpp-base64 STATIC
    include/cppbase64/base64.cpp
    include/cppbase64/base64.h)

target_include_directories(jsoncpp_lib PUBLIC include/jsoncpp)
target_include_directories(cpp-base64 PUBLIC include/cppbase64)

find_package(CURL QUIET)
if(NOT CURL_FOUND AND VCPKG_ROOT)
    message(STATUS "System curl not found, trying vcpkg...")
    file(GLOB _vcpkg_installed
        "${VCPKG_ROOT}/installed/*/share/curl/curlConfig.cmake"
        "${VCPKG_ROOT}/installed/*/share/curl/CURLConfig.cmake"
    )
    if(_vcpkg_installed)
        list(GET _vcpkg_installed 0 _cfg)
        cmake_path(GET _cfg PARENT_PATH _share)     
        cmake_path(GET _share PARENT_PATH _share)    
        cmake_path(GET _share PARENT_PATH _prefix)   
        set(CMAKE_PREFIX_PATH "${_prefix}")
        find_package(curl CONFIG REQUIRED)
    else()
        message(FATAL_ERROR
            "curl not found.\n"
            "  System: apt install libcurl-dev / brew install curl / dnf install libcurl-devel\n"
            "  vcpkg:  vcpkg install curl"
        )
    endif()
elseif(NOT CURL_FOUND)
    message(FATAL_ERROR
        "curl not found. Install it:\n"
        "  apt install libcurl-dev   (Debian/Ubuntu)\n"
        "  brew install curl         (macOS)\n"
        "  dnf install libcurl-devel (Fedora)\n"
        "  vcpkg install curl        (cross-platform)"
    )
endif()

add_executable(ApiSender src/ApiSender.cpp src/OptionalFeatures.h "src/Apisenderh.h" "include/cppbase64/base64.cpp" "include/cppbase64/base64.h")
target_link_libraries(ApiSender PRIVATE CURL::libcurl jsoncpp_lib cpp-base64)
