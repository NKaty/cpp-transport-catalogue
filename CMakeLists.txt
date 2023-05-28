cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(TEST_MODE)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport-catalogue/transport_catalogue.proto
        transport-catalogue/map_renderer.proto
        transport-catalogue/svg.proto
        transport-catalogue/graph.proto
        transport-catalogue/transport_router.proto)

set(TC_FILES transport-catalogue/geo.h
        transport-catalogue/input_reader.cpp
        transport-catalogue/input_reader.h
        transport-catalogue/main.cpp
        transport-catalogue/stat_reader.cpp
        transport-catalogue/stat_reader.h
        transport-catalogue/transport_catalogue.cpp
        transport-catalogue/transport_catalogue.h
        transport-catalogue/domain.h
        transport-catalogue/domain.cpp
        transport-catalogue/geo.cpp
        transport-catalogue/json.cpp
        transport-catalogue/request_handler.cpp
        transport-catalogue/request_handler.h
        transport-catalogue/json_reader.cpp
        transport-catalogue/json_reader.h
        transport-catalogue/svg.cpp
        transport-catalogue/svg.h
        transport-catalogue/map_renderer.h
        transport-catalogue/map_renderer.cpp
        transport-catalogue/testing_library.h
        transport-catalogue/test_transport_catalogue.cpp
        transport-catalogue/test_input_reader.cpp
        transport-catalogue/test_stat_reader.cpp
        transport-catalogue/test_json_reader.cpp
        transport-catalogue/test_map_renderer.cpp
        transport-catalogue/test_request_handler.cpp
        transport-catalogue/test_json.cpp
        transport-catalogue/test_svg.cpp
        transport-catalogue/test_geo.cpp
        transport-catalogue/json_builder.h
        transport-catalogue/json_builder.cpp
        transport-catalogue/test_json_builder.cpp
        transport-catalogue/graph.h
        transport-catalogue/ranges.h
        transport-catalogue/router.h
        transport-catalogue/transport_router.h
        transport-catalogue/transport_router.cpp
        transport-catalogue/test_transport_router.cpp
        transport-catalogue/test_graph.cpp
        transport-catalogue/test_router.cpp
        transport-catalogue/test_router.cpp
        transport-catalogue/test_ranges.cpp
        transport-catalogue/serialization.cpp
        transport-catalogue/serialization.h transport-catalogue/test_serialization.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TC_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
