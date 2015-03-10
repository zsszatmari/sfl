# building libcore
# ./bcp boost/shared_ptr.hpp boost/move/unique_ptr.hpp boost/enable_shared_from_this.hpp boost/signals2.hpp boost/date_time/gregorian/gregorian.hpp boost/date_time/posix_time/posix_time.hpp boost/filesystem/convenience.hpp boost/thread/once.hpp boost/thread/thread.hpp boost/interprocess/sync/file_lock.hpp boost/thread/condition_variable.hpp boost/thread/mutex.hpp boost/random.hpp boost/thread.hpp boost/unordered_set.hpp boost/unordered_map.hpp libs/chrono/src libs/system/src/error_code.cpp boost/chrono/detail/inlined/process_cpu_clocks.hpp boost/asio.hpp boost/asio/ssl.hpp boost/variant.hpp boost/atomic/detail/caps_gcc_atomic.hpp boost/atomic/detail/ops_gcc_atomic.hpp ../bcp

cmake_minimum_required (VERSION 2.6)
project (libcore)

if (WIN32 AND NOT MINGW)
#if (WIN32)
	SET (BUILD_SHARED_LIBS 1)
endif ()

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -O0")


# obviously OS_MACOSX only makes sense on macosx, but needed by leveldb when compiling on osx

set (CMAKE_C_FLAGS "-DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS4_UNICODE61")
if (APPLE)
	if (OSX_VERSION)
		add_custom_target(CopyToMac ALL cp libcore.a .. && cd .. && pwd && sh copytomac.sh
						  DEPENDS core)

		MESSAGE("os x version")
		#TODO: optimization!
		set ( CMAKE_CXX_FLAGS "-DGTEST_LANG_CXX11=0 -std=gnu++1y -stdlib=libc++ -mmacosx-version-min=10.7 -fobjc-arc -DLEVELDB_PLATFORM_POSIX -DOS_MACOSX" )
		file (GLOB core_EXTRA
			  "core/Apple/*.mm"
			  "core/Apple/*.cpp"
			  "core/Base/leveldb/port/port_posix.cc"
			  "core/Base/leveldb/util/env_posix.cc"
			  "core/boost/thread/src/pthread/*.cpp"
			  "core/boost/chrono/src/*.cpp")
		file (GLOB core_EXTRAINC
			   core/public/Apple
			   core/Apple)
			   
		SET_SOURCE_FILES_PROPERTIES(core/Apple/AppleHttpDownloader.mm PROPERTIES COMPILE_FLAGS -fno-objc-arc)
		SET_SOURCE_FILES_PROPERTIES(core/Apple/PlaybackWorker.mm PROPERTIES COMPILE_FLAGS -fno-objc-arc)
	else ()
		add_custom_target(CopyToQt ALL cp libcore.a .. && cd .. && pwd && sh copytoqtmac.sh
						  DEPENDS core)

		MESSAGE("osx/qt version")
		set ( CMAKE_CXX_FLAGS "-std=gnu++1y -stdlib=libc++ -mmacosx-version-min=10.7 -DSQLITE_ENABLE_FTS3 -DNON_APPLE -DOS_MACOSX" )
		file (GLOB core_EXTRA
			  "core/Base/leveldb/port/port_posix.cc"
			  "core/Base/leveldb/util/env_posix.cc"
			  "core/boost/thread/src/pthread/*.cpp")

		#file (GLOB core_EXTRAINC
		#	  dummy)
	endif ()
elseif (WIN32)
	if (MINGW)
		#set ( CMAKE_CXX_FLAGS "-std=c++11 -DNON_APPLE -DLEVELDB_PLATFORM_WINDOWS -DOS_WIN -DWIN32" )
		set ( CMAKE_CXX_FLAGS "-std=c++11 -DSQLITE_ENABLE_FTS3 -DWIN32 -DNON_APPLE")
		#-DBOOST_THREAD_BUILD_LIB -DBOOST_ALL_NO_LIB -DBUILDING_CORE_DLL -DJSON_DLL_BUILD" )
		
		set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L../lib/win" )
		
		#leveldb does not compile as posix
		file (GLOB core_EXTRA
	          "core/Base/leveldb/port/port_win.cc"
		  "core/Base/leveldb/util/env_boost.cc"
		  #"core/boost/thread/src/pthread/*.cpp"
		  )
		file (GLOB core_EXTRAINC
			  dummy)

		add_custom_target(CopyToQt ALL copy libcore.a .. && cd .. && copytoqtwin.bat
						  DEPENDS core)
	else (MINGW)
		set ( CMAKE_C_FLAGS "-DBUILDING_CORE_DLL")
		set ( CMAKE_CXX_FLAGS "/DNON_APPLE /DSQLITE_ENABLE_FTS3 /DOS_WIN /DWIN32 /EHsc /DBOOST_THREAD_BUILD_LIB /DBOOST_ALL_NO_LIB /DBUILDING_CORE_DLL /DJSON_DLL_BUILD /D_WIN32_WINNT=0x0601" )

		set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
		file (GLOB core_EXTRA
	          "core/Base/leveldb/port/port_win.cc"
		  "core/Base/leveldb/util/env_boost.cc"
		  "core/Base/leveldb/util/win_logger.cc"
		  "core/boost/thread/src/win32/*.cpp")
		file (GLOB core_EXTRAINC
			  dummy)
	endif ()
else ()
	set ( CMAKE_CXX_FLAGS "-std=c++11 -DSQLITE_ENABLE_FTS3 -DNON_APPLE" )
	file (GLOB core_EXTRA
		  "core/Base/leveldb/port/port_posix.cc"
		  "core/Base/leveldb/util/env_posix.cc"
		  "core/boost/thread/src/pthread/*.cpp")
	file (GLOB core_EXTRAINC
			  dummy)
endif ()

file(GLOB core_SRC
	"external/clientdb/*.cpp"
	"external/clientdb/sqlite/sqlite3.c"
	"core/*.h"
	"core/*.c"
	"core/*.cpp"
	"core/jsoncpp/*.cpp"
	"core/Base/*.cpp"
	"core/Gui/*.cpp"
	"core/Cast/*.cpp"
	"core/Cast/*.cc"
	"core/vorbis/*.c"
	"core/ogg/*.c"
	"core/boost/smart_ptr/src/*.cpp"
	"core/boost/date_time/src/gregorian/*.cpp"
	"core/boost/date_time/src/posix_time/*.cpp"
	"core/boost/exception/src/*.cpp"
	"core/boost/filesystem/src/*.cpp"
	"core/boost/system/src/*.cpp"
	"core/google/protobuf/*.cc"
	"core/google/protobuf/io/*.cc"
	"core/google/protobuf/stubs/*.cc"
)
# only ios: 
# "core/boost/atomic/src/*.cpp"
# "core/boost/thread/src/*.cpp"

include_directories (
		core
		core/public
		core/public/Base
		core/Apple
		core/Base/leveldb
		core/Base/leveldb/include
		core/public/Gui 
		core/jsoncpp
		core/Base
		core/Cast
		core/vorbis
		core/google
		~/projects/gmock-1.7.0/include
		~/projects/gmock-1.7.0/gtest/include
		external/clientdb
		external/clientdb/sqlite
		include
		${core_EXTRAINC}
		)

add_library (core ${core_SRC} ${core_EXTRA})
if (MINGW)
    target_link_libraries(core ssleay32)
    target_link_libraries(core libeay32)
    target_link_libraries(core ws2_32)
    target_link_libraries(core wsock32)
    target_link_libraries(core avcodec-56)
    target_link_libraries(core avformat-56)
    target_link_libraries(core avutil-54)
    target_link_libraries(core avresample-2)
    target_link_libraries(core avfilter-5)
    target_link_libraries(core swresample-1)
    target_link_libraries(core mpg123-0)
endif()

if (WIN32)
	# no testing on win32
else()
	FILE (GLOB test_src
		  "tests/*.cpp"
		  "/Users/zsszatmari/projects/gmock-1.7.0/fused-src/gmock-gtest-all.cc")

add_executable(test ${test_src})
set(CMAKE_EXE_LINKER_FLAGS "-framework SystemConfiguration -framework Security -L../platforms/mac/lib/x64 -lssl -lcrypto -lavcodec.56.13.100 -lavformat.56.15.102 -lavresample.2.1.0 -lavutil.54.15.100 -lavfilter.5.2.103 -lmpg123")

	target_link_libraries (test core)
	add_custom_target(RunTests ALL ./test
							  DEPENDS test)
endif()
