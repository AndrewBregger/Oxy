cmake_minimum_required(VERSION 3.0)

project(oxc)

#set(CMAKE_C_STANDARD 11)

set(CC /usr/local/bin/gcc)
# set(CXX /usr/local/bin/g++)

set(INCLUDE_DIR "src")

include_directories(${INCLUDE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -pedantic -Wnested-anon-types -std=c11")

set(SOURCE main.c src/io.c src/common.c src/token.c src/lex.c src/print.c src/oxy.c
           src/ast.c src/parser.c src/report.c
           src/value.c src/checker.c
           src/scope.c src/entity.c src/type.c)

add_executable(oxc ${SOURCE})
