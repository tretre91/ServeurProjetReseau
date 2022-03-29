add_subdirectory(broadcast)

add_executable(server server.cpp client.cpp main.cpp)
target_link_libraries(server PUBLIC bluetooth dasynq fmt::fmt broadcast proto pthread)
