
add_executable(rtmp_server_dev EXCLUDE_FROM_ALL
    main.cpp
    # server.cpp
    server.cpp
    listener.cpp
    connection.cpp
)

add_dependencies(rtmp_server_dev
    common
)

message("targe link rtmp_server")
target_link_libraries(rtmp_server_dev
    common
)
