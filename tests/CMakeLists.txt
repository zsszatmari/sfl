cmake_minimum_required (VERSION 2.6)
project (sfltest)

file (GLOB test_src *.cpp)

include_directories (
	../../
	~/projects/boost57
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(Tests ${test_src})
if (WIN32)
	add_custom_target(RunTests ALL Debug/test
							   DEPENDS test)
else ()
	add_custom_target(RunTests ALL ./test
							   DEPENDS test)
endif ()

