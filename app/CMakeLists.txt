
add_executable(rtmp_server_dev EXCLUDE_FROM_ALL
    main.cpp
    # server.cpp
    rtmp_server.cpp
)

add_dependencies(rtmp_server_dev
    thread
    io
    log
    config
)

message("targe link rtmp_server")
target_link_libraries(rtmp_server_dev
    thread
    log
    io
    config
    st
)
