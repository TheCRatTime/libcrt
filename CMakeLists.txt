cmake_minimum_required(VERSION 3.10)

project(libcrt C)

add_executable(atexit      examples/atexit.c)
add_executable(arena       examples/arena.c)
add_executable(hex_example examples/hex_example.c)
add_executable(rle_encode  examples/rle_encode.c)
add_executable(block_alloc examples/block_alloc.c)
add_executable(dbg_alloc   examples/dbg_alloc.c)
add_executable(GCmalloc    examples/GCmalloc.c)
add_executable(CErrors     examples/CErrors.c)
