cmake_minimum_required(VERSION 3.29.6)

SET(PROJECT "PythonIntegration")

project(${PROJECT} C)

set(CMAKE_C_STANDARD 23)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")

add_executable(${PROJECT} main.c
        name.c
        name.h
        LAN/LAN.c
        LAN/LAN.h
        LAN/crossplatform.h
        JEU/jeu.c
        JEU/jeu.h
        JEU/structures.h
        version.c
        version.h
        GRAPHISMES/graphismes.c
        GRAPHISMES/graphismes.h
)


if(WIN32)
    target_link_libraries(${PROJECT} ws2_32)
endif()


target_link_libraries(${PROJECT} -lmingw32 -lalleg44)