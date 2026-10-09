add_executable(receiver
    main.cpp
    MainWindow.h
    MainWindow.cpp
    UdpWorker.h
    UdpWorker.cpp
)

target_link_libraries(receiver PRIVATE common 
  Qt6::Network
  Qt6::Widgets 
  Qt6::Core
)