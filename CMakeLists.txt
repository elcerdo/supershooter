project(supershooter)
cmake_minimum_required(VERSION 2.6)

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_mixer REQUIRED)
find_package(OpenGL REQUIRED)

add_definitions(-DTIXML_USE_STL)
subdirs(tinyxml)

add_library(zizi
    engine.cpp
    sprite.cpp
    sound.cpp
    collision.cpp
    message.cpp
    shoot.cpp
    utils.cpp
    except.cpp)
target_link_libraries(zizi
    boost_regex
    tinyxml
    ${SDL_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${OPENGL_gl_LIBRARY})

add_executable(zizidemo
    zizidemo.cpp)
target_link_libraries(zizidemo
    zizi)

add_executable(zizicollision
    zizicollision.cpp)
target_link_libraries(zizicollision
    zizi)

add_executable(supershooter
    main.cpp)
target_link_libraries(supershooter
    zizi)

install(TARGETS supershooter
        RUNTIME DESTINATION bin)
install(DIRECTORY data
        DESTINATION share/supershooter)
install(FILES config.xml
        DESTINATION share/supershooter)
install(FILES supershooter.desktop
        DESTINATION share/applications)
install(FILES supershooter.xpm
        DESTINATION share/pixmaps)

