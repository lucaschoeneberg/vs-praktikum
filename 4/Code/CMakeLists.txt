cmake_minimum_required(VERSION 3.13)

project(pub_sub_system)

set(CMAKE_CXX_STANDARD 14)

# Add the directory with FindProtobuf.cmake and FindGRPC.cmake to CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

# Find required protobuf
find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

# Find required gRPC
find_package(GRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

# Include generated *.pb.h files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Generate the .pb.cc and .pb.h files from .proto files
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})
    list(APPEND PROTO_SRCS ${PROTO_SRCS})
    list(APPEND PROTO_HDRS ${PROTO_HDRS})

    # Generate the .grpc.pb.cc and .grpc.pb.h files from .proto files
    grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_FILE})
    list(APPEND GRPC_SRCS ${GRPC_SRCS})
    list(APPEND GRPC_HDRS ${GRPC_HDRS})
endforeach()

add_executable(pub_sub_server
        ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
        pub_sub_server.cpp
        )

add_executable(pub_sub_receiver
        ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
        pub_sub_receiver.cpp
        )

add_executable(pub_sub_client
        ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
        pub_sub_client.cpp
        )

target_link_libraries(pub_sub_server
        ${Protobuf_LIBRARIES}
        gRPC::grpc++
        gRPC::grpc++_reflection
        pthread
        dl
        )

target_link_libraries(pub_sub_receiver
        ${Protobuf_LIBRARIES}
        gRPC::grpc++
        gRPC::grpc++_reflection
        pthread
        dl
        )

target_link_libraries(pub_sub_client
        ${Protobuf_LIBRARIES}
        gRPC::grpc++
        gRPC::grpc++_reflection
        pthread
        dl
        )
