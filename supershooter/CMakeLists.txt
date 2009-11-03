
add_executable(supershooter
    shoot.cpp
    main.cpp)
target_link_libraries(supershooter
    tinyxml
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

