cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto map_renderer.proto svg.proto graph.proto transport_router.proto)

find_package(Threads REQUIRED)

set(TRANSPORT_CATALOGUE_FILES main.cpp
	graph.h ranges.h router.h 
	domain.h domain.cpp 
	geo.h geo.cpp 
	json.h json.cpp 
	json_builder.h json_builder.cpp 
	json_reader.h json_reader.cpp 
	map_renderer.h map_renderer.cpp 
	request_handler.h request_handler.cpp 
	serialization.h serialization.cpp 
	svg.h svg.cpp 
	transport_catalogue.h transport_catalogue.cpp 
	transport_router.h transport_router.cpp
)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(transport_catalogue ${Protobuf_LIBRARIES} Threads::Threads)