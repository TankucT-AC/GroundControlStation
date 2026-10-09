add_executable(sender main.cpp)
target_link_libraries(sender PRIVATE common Qt6::Network Qt6::Core)