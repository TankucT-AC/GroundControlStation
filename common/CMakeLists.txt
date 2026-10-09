add_library(common STATIC
    TelemetryPacket.cpp
    TelemetryPacket.h
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PUBLIC Qt6::Core)