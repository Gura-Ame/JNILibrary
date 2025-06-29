cmake_minimum_required(VERSION 3.31)
project(JNILibrary)

set(CMAKE_CXX_STANDARD 23)

# 找到 Java 與 JNI
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# 印出 JNI 資訊
message(STATUS "JNI include dirs: ${JNI_INCLUDE_DIRS}")
message(STATUS "JNI libraries: ${JNI_LIBRARIES}")

# 設定輸出目錄為 .c 所在目錄
set(SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(OUTPUT_DIR ${SOURCE_DIR})

# 建立 DLL
add_library(org_example_Native SHARED org_example_Native.cpp)

# 設定 include path
target_include_directories(org_example_Native PRIVATE ${JNI_INCLUDE_DIRS})

# 若是 Windows，加上編譯定義
if (WIN32)
    target_compile_definitions(org_example_Native PRIVATE -D_JNI_IMPLEMENTATION_)
endif()

# 移除 Windows 上的 lib 前綴
set_target_properties(org_example_Native PROPERTIES
        OUTPUT_NAME "org_example_Native"
        PREFIX ""  # no "lib" prefix
)

# 設定輸出到與 .c 同目錄
set_target_properties(org_example_Native PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # Windows DLL
        ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # .lib / .a
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # .so / .dylib
)

add_custom_command(TARGET org_example_Native POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_SOURCE_DIR}/liborg_example_Native.dll.a")
