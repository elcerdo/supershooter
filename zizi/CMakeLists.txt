add_library(zizi
    engine.cpp
    sprite.cpp
    sound.cpp
    collision.cpp
    message.cpp
    gui.cpp
    utils.cpp
    except.cpp)
target_link_libraries(zizi
    boost_regex
    ${SDL_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${OPENGL_gl_LIBRARY})
