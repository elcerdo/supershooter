
add_executable(supershooter
    shoot.cpp
    main.cpp)
target_link_libraries(supershooter
    tinyxml
    zizi)

install(TARGETS supershooter
        RUNTIME DESTINATION bin)
