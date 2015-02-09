# building libcore
# ./bcp boost/shared_ptr.hpp boost/enable_shared_from_this.hpp boost/signals2.hpp boost/date_time/gregorian/gregorian.hpp boost/date_time/posix_time/posix_time.hpp boost/filesystem/convenience.hpp boost/thread/once.hpp boost/thread/thread.hpp boost/interprocess/sync/file_lock.hpp boost/thread/condition_variable.hpp boost/thread/mutex.hpp boost/random.hpp boost/interprocess/smart_ptr/unique_ptr.hpp boost/thread.hpp boost/unordered_set.hpp boost/unordered_map.hpp libs/chrono/src boost/chrono/detail/inlined/process_cpu_clocks.hpp boost/asio.hpp boost/asio/ssl.hpp ../bcp

cmake_minimum_required (VERSION 2.6)
project (clientdb)

# obviously OS_MACOSX only makes sens on macosx, but needed by leveldb when compiling on osx

set ( CMAKE_C_FLAGS "-DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS4_UNICODE61")
set ( CMAKE_CXX_FLAGS "-DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS4_UNICODE61 -DGTEST_LANG_CXX11=0 -std=c++1y -stdlib=libc++ -mmacosx-version-min=10.9 -DLEVELDB_PLATFORM_POSIX -DOS_MACOSX" )
        
set ( CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g" )

include_directories (
		.
        jsoncpp
        stdplus
		leveldb
		leveldb/include
        ~/projects/gmock-1.7.0/include
        ~/projects/gmock-1.7.0/gtest/include
		)

file (GLOB db_src
      "*.cpp"
      "jsoncpp/*.cpp"
      "sqlite/sqlite3.c"
      leveldb/db/builder.cc
	leveldb/db/db_impl.cc
	leveldb/db/db_iter.cc
	leveldb/db/dbformat.cc
	leveldb/db/filename.cc
	leveldb/db/log_reader.cc
	leveldb/db/log_writer.cc
	leveldb/db/memtable.cc
	leveldb/db/repair.cc
	leveldb/db/version_edit.cc
	leveldb/db/version_set.cc
	leveldb/db/write_batch.cc
	leveldb/db/table_cache.cc
	leveldb/table/block_builder.cc
	leveldb/table/block.cc
	leveldb/table/filter_block.cc
	leveldb/table/format.cc
	leveldb/table/iterator.cc
	leveldb/table/merger.cc
	leveldb/table/table_builder.cc
	leveldb/table/table.cc
	leveldb/table/two_level_iterator.cc
	leveldb/util/arena.cc
	leveldb/util/bloom.cc
	leveldb/util/cache.cc
	leveldb/util/coding.cc
	leveldb/util/comparator.cc
	leveldb/util/crc32c.cc
	leveldb/util/env.cc
	leveldb/util/filter_policy.cc
	leveldb/util/hash.cc
	leveldb/util/histogram.cc
	leveldb/util/logging.cc
	leveldb/util/options.cc
	leveldb/util/status.cc
	leveldb/port/port_posix.cc
	leveldb/util/env_posix.cc)

add_library (clientdb ${db_src})

FILE (GLOB test_src
      "tests/*.cpp"
      "/Users/zsszatmari/projects/gmock-1.7.0/fused-src/gmock-gtest-all.cc")

add_executable(test ${test_src})
target_link_libraries (test clientdb)

add_custom_target(RunTests ALL ./test
						  DEPENDS test)
