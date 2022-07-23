cmake_minimum_required(VERSION 3.23)

# set(CMAKE_VERBOSE_MAKEFILE ON)

project(compiler)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin")
add_executable(compiler src/compiler.c)

if(APPLE)
  execute_process(COMMAND brew --prefix libgccjit OUTPUT_VARIABLE LIBGCCJIT_DIR)
  execute_process(COMMAND brew --prefix json-c OUTPUT_VARIABLE JSON_C_DIR)
  string(STRIP ${LIBGCCJIT_DIR} LIBGCCJIT_DIR)
  string(STRIP ${JSON_C_DIR} JSON_C_DIR)

  target_include_directories(compiler PRIVATE ${LIBGCCJIT_DIR}/include)
  target_include_directories(compiler PRIVATE ${JSON_C_DIR}/include)
  target_link_directories(compiler PRIVATE ${LIBGCCJIT_DIR}/lib/gcc/11)
  target_link_directories(compiler PRIVATE ${JSON_C_DIR}/lib)
endif()

target_link_libraries(compiler gccjit json-c)

