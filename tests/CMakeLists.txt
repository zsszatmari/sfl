cmake_minimum_required (VERSION 2.6)
project (test)

include_directories (
	../core/public 
	../core/public/Base
	../core/
	../core/Base
	../include
	~/projects/gmock-1.7.0/include
	~/projects/gmock-1.7.0/gtest/include
)

#set ( CMAKE_CXX_FLAGS "/DBOOST_ALL_NO_LIB /DWIN32 /EHsc")
set ( CMAKE_CXX_FLAGS "-DGTEST_LANG_CXX11=0 -std=gnu++1y -stdlib=libstdc++ -mmacosx-version-min=10.6 -fobjc-arc -DLEVELDB_PLATFORM_POSIX -DOS_MACOSX" )

#set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")

FILE (GLOB test_src
	  "*.cpp"
	  "/Users/zsszatmari/projects/gmock-1.7.0/fused-src/gmock-gtest-all.cc")

add_executable(tests ${test_src})
#find_library(CORE_LIB NAMES core
#	  HINTS "../libcore.a")
set(CMAKE_EXE_LINKER_FLAGS "-L../debug -lcore -framework SystemConfiguration -framework Security -L/Users/zsszatmari/projects/G-Ear-v2/lib/x64 -lssl -lcrypto -lavcodec.56.1.0 -lavformat.56.1.0 -lavresample.2.1.0 -lavutil.54.3.0 -lmpg123")

#target_link_libraries(tests CORE_LIB)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ../)

