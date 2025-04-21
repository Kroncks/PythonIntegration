cmake_minimum_required(VERSION 3.29.6)
project(PythonIntegration C)

set(CMAKE_C_STANDARD 23)

add_executable(PythonIntegration main.c
        name.c
        name.h
        LAN/LAN.c
        LAN/LAN.h
        LAN/crossplatform.h
)
