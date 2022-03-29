add_library(broadcast OBJECT broadcast.c broad.c)
target_include_directories(broadcast PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
