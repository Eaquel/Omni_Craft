cmake_minimum_required(VERSION 4.4.2)

project("Omni_Craft")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Omni_Craft SHARED
    Craft.cpp
)

target_include_directories(Omni_Craft PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_library(log-lib log)
find_library(android-lib android)
find_library(jnigraphics-lib jnigraphics)
find_library(gles-lib GLESv3)
find_library(egl-lib EGL)

target_compile_options(Omni_Craft PRIVATE
    -O3
    -flto
    -ffast-math
    -funroll-loops
    -fomit-frame-pointer
    -fvisibility=hidden
)

target_link_libraries(Omni_Craft
    ${log-lib}
    ${android-lib}
    ${jnigraphics-lib}
    ${gles-lib}
    ${egl-lib}
)
