add_executable(server server.cpp)
target_link_libraries(server PUBLIC bluetooth fmt::fmt asio)

add_executable(colin server_colin.c)
target_link_libraries(colin PUBLIC bluetooth)
