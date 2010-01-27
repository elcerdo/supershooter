
add_executable(blocks
    uct.cpp
    board.cpp
    boardblocks.cpp
    main.cpp)
target_link_libraries(blocks
    zizi)

install(TARGETS blocks
        RUNTIME DESTINATION bin)
