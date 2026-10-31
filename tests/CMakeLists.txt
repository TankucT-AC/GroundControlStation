enable_testing()

add_executable(test_telemetry test_telemetry.cpp)
target_link_libraries(test_telemetry PRIVATE common Qt6::Test Qt6::Core)

add_test(NAME test_telemetry COMMAND test_telemetry)

add_executable(test_udpworker
    test_udpworker.cpp
    ../receiver/UdpWorker.cpp
    ../receiver/UdpWorker.h
)
target_link_libraries(test_udpworker PRIVATE common Qt6::Test Qt6::Network Qt6::Core)
target_include_directories(test_udpworker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../receiver)

add_test(NAME test_udpworker COMMAND test_udpworker)

add_executable(test_theme test_theme.cpp)
target_link_libraries(test_theme PRIVATE Qt6::Test Qt6::Gui)
target_include_directories(test_theme PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../receiver)

add_test(NAME test_theme COMMAND test_theme)