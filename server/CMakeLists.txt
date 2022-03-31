add_executable(server broadcast.c server.cpp client.cpp main.cpp)
target_link_libraries(server PUBLIC bluetooth dasynq fmt::fmt proto)
