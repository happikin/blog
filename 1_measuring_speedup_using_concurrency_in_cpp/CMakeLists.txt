cmake_minimum_required(VERSION 3.10)
project(hpc)

add_executable(moderndemo ${CMAKE_CURRENT_SOURCE_DIR}/modern-demo.cc)
target_link_libraries(moderndemo pthread)
# add_executable(simpledemo ${CMAKE_CURRENT_SOURCE_DIR}/simple-demo.cc)
# target_link_libraries(simpledemo pthread)