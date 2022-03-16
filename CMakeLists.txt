cmake_minimum_required(VERSION 3.14)

project(
  FBXOS
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

add_subdirectory(server)
add_subdirectory(client)