cmake_minimum_required(VERSION 3.24)
project(3D_Graphics)

set(CMAKE_CXX_STANDARD 17)

include_directories(include /opt/homebrew/include/SDL2)
add_compile_options(-Wall)
add_library(console-game-engine
        include/SimpleGameEngine.h
        include/SimpleGameEngine.cpp)
target_link_libraries(console-game-engine -lSDL2)
target_link_libraries(console-game-engine -L/opt/homebrew/lib/)
add_executable(3D_Graphics src/main.cpp)
target_link_libraries(3D_Graphics console-game-engine)