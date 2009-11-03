
add_executable(zizidemo
    zizidemo.cpp)
target_link_libraries(zizidemo
    zizi)

add_executable(zizicollision
    ../supershooter/shoot.cpp
    zizicollision.cpp)
target_link_libraries(zizicollision
    tinyxml
    zizi)

