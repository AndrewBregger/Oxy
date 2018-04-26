cmake_minimum_required(VERSION 3.0)

project(co)

set(CMAKE_CXX_STANDARD 17)

set(CC /usr/local/bin/gcc)
set(CXX /usr/local/bin/g++)

set(INCLUDE_DIR "src")

include_directories(${INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -pedantic -Wnested-anon-types")

set(SOURCE main.cpp src/io.cpp src/pratt.cpp src/token.cpp src/parser.cpp
    src/print.cpp src/lex.cpp src/ast.cpp)

add_executable(pratt ${SOURCE})
