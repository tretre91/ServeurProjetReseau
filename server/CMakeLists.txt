add_executable(server server.cpp main.cpp)
target_link_libraries(server PUBLIC bluetooth dasynq fmt::fmt)
