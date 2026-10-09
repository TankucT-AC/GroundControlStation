enable_testing()

add_executable(test_telemetry test_telemetry.cpp)
target_link_libraries(test_telemetry PRIVATE common Qt6::Test Qt6::Core)

add_test(NAME test_telemetry COMMAND test_telemetry)