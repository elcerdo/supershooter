project(supershooter)
cmake_minimum_required(VERSION 2.6)

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(OpenGL REQUIRED)

add_library(zizi
    engine.cpp
    shoot.cpp
    except.cpp)
target_link_libraries(zizi
    boost_regex
    ${SDL_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${OPENGL_gl_LIBRARY})

add_executable(zizidemo
    zizidemo.cpp)
target_link_libraries(zizidemo
    zizi)

add_executable(supershooter
    main.cpp)
target_link_libraries(supershooter
    zizi)
