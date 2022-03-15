add_executable(server main.cpp)
target_link_libraries(server PUBLIC btlib fmt::fmt)

file(COPY device.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
