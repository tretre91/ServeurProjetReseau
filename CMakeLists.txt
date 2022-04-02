cmake_minimum_required(VERSION 3.14)

project(
  PROJET_RESEAU
  LANGUAGES C CXX
  VERSION 0.0.1
  DESCRIPTION ""
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(CPM)

CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY "fmtlib/fmt"
  GIT_TAG "8.0.0"
  OPTIONS "FMT_DOC OFF" "FMT_TEST OFF"
)

CPMAddPackage(
  NAME dasynq
  GITHUB_REPOSITORY "davmac314/dasynq"
  GIT_TAG "v1.2.2"
  DOWNLOAD_ONLY YES
)

if (dasynq_ADDED)
  add_library(dasynq INTERFACE)
  target_include_directories(dasynq INTERFACE "${dasynq_SOURCE_DIR}/include")
else()
  message(FATAL_ERROR "ERROR: failed to add dasynq")
endif()

add_subdirectory(message)

add_executable(server src/broadcast.c src/server.cpp src/client.cpp src/main.cpp)
target_link_libraries(server PUBLIC bluetooth dasynq fmt::fmt proto)
