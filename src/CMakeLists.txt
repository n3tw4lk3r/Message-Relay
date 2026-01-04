add_library(Message-Relay STATIC
    core/Client.c
    core/DisplayServer.c
    net/tcpServer.c
    utils/parse.c
    utils/safe_io.c
)

target_include_directories(Message-Relay
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

add_executable(run_Client
    executables/run_Client.c
)

target_link_libraries(run_Client
    PRIVATE Message-Relay 
)

add_executable(run_DisplayServer
    executables/run_DisplayServer.c
)

target_link_libraries(run_DisplayServer
  PRIVATE Message-Relay 
)
