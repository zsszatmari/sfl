cmake_minimum_required (VERSION 2.6)
project (sfltest)

file (GLOB test_src *.cpp)

include_directories (
	../../
	~/projects/boost57
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(test ${test_src})
add_custom_target(RunTests ALL ./test
						   DEPENDS test)
