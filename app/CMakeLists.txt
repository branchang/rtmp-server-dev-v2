
add_executable(rtmp_server_dev EXCLUDE_FROM_ALL
    main.cpp
    connection.cpp
    listener.cpp
    recv_thread.cpp
    server.cpp
    rtmp_connection.cpp
)

add_dependencies(rtmp_server_dev
    common
    protocol
)

message("targe link rtmp_server")
target_link_libraries(rtmp_server_dev
    common
    protocol
)
