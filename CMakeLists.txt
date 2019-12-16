cmake_minimum_required(VERSION 3.13)

project(stream)

add_library(stream INTERFACE)

add_subdirectory(include)
add_subdirectory(src)

target_compile_features(stream INTERFACE cxx_std_17)

#Set(CMAKE_CXX_FLAGS "-Wall" "-Wextra")

install(TARGETS stream EXPORT stream DESTINATION lib/stream)
install(EXPORT stream DESTINATION lib/stream)
install(FILES stream-config.cmake DESTINATION lib/stream)

find_Package(reflection REQUIRED)