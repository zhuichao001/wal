cmake_minimum_required(VERSION 3.10)

project(wal C CXX)

set(CXX_FLAGS
 -g
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 -std=c++14
 -rdynamic
 )

include_directories(../../naughty/fio)
#add_subdirectory(../../naughty/fio, fio_lib_dir)

aux_source_directory(. DIR_LIB_WAL_SRCS)
add_library(wal STATIC ${DIR_LIB_WAL_SRCS})
