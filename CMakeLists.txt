cmake_minimum_required(VERSION 3.30)
project(PythonIntegration C)

set(CMAKE_C_STANDARD 23)

add_executable(PythonIntegration main.c
        name.c
        name.h
        LAN/LAN.c
        LAN/LAN.h
        LAN/crossplatform.h
)


if(WIN32)
    target_link_libraries(PythonIntegration ws2_32)
endif()
