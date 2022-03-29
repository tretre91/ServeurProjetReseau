find_package(Protobuf REQUIRED)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/message.pb.h" "${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc"
  COMMAND protobuf::protoc ARGS "-I=${CMAKE_CURRENT_SOURCE_DIR}" "--cpp_out=${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/message.proto"
  MAIN_DEPENDENCY "message.proto"
)

add_library(proto OBJECT "${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc")
target_include_directories(proto INTERFACE ${Protobuf_INCLUDE_DIRS})
target_include_directories(proto INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(proto INTERFACE ${Protobuf_LIBRARIES})
