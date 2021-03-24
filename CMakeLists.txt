cmake_minimum_required(VERSION 3.5)

project(rtmp_server_dev)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

find_package(Git)

execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=6 --dirty --always --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE  GIT_REPO_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=short
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE  GIT_REPO_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%H
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE  GIT_REPO_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

file(WRITE ${PROJECT_BINARY_DIR}/repo_version.h
    "#define REPO_VERSION \"${GIT_REPO_VERSION}\"\n#define REPO_DATE \"${GIT_REPO_DATE}\"\n#define REPO_HASH \"${GIT_REPO_HASH}\"\n"
)

include_directories(${PROJECT_BINARY_DIR}
    "/usr/local/Cellar/llvm/10.0.0_3/lib/clang/10.0.0/include"
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include"
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    "/Users/Bran/Code/sourcecode/Media/rtmp-server-lam2003/build/st-1.9/DARWIN_19.6.0_OPT"
)

link_directories(
    ${PROJECT_BINARY_DIR}/lib
    ${PROJECT_BINARY_DIR}/common
    ${PROJECT_BINARY_DIR}/protocol
)

message(${PROJECT_BINARY_DIR})


# include(3rdparty.cmake)
add_subdirectory(common)
message("make servers !!!!")
add_subdirectory(protocol)
add_subdirectory(app)
