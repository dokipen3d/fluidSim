project(fluidSim)
cmake_minimum_required(VERSION 2.8)


message("system: ${CMAKE_SYSTEM_PREFIX_PATH}")
set(CMAKE_C_COMPILER "/usr/local/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")

set(LIBRARY_SEARCH_PATH
  /usr/local
  /opt/local
  /usr
  /opt
  /Users/dokipen/code/openmp/runtime/exports/mac_32e/lib
)

#find_library(IOMP5_LIBRARY iomp5 ${LIBRARY_SEARCH_PATH})
#MESSAGE(STATUS "Found iomp5 in: ${IOMP5_LIBRARY}")

#find_library(IOMP5_LIBRARIES iomp5 ${LIBRARY_SEARCH_PATH})
#MESSAGE(STATUS "Found iomp5 in: ${IOMP5_LIBRARIES}")


#set(CMAKE_C_COMPILER /Users/dokipen/code/llvm/build/Release/bin/clang2)
#set(CMAKE_CXX_COMPILER /Users/dokipen/code/llvm/build/Release/bin/clang2++)

#include_directories(/Users/dokipen/code/llvm/build/include/c++/v1)
#include_directories(/Users/dokipen/code/openmp/runtime/build)

#-lstdc++ -stdlib=libc++
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -std=c++1y -pedantic -mfpmath=sse -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -fopenmp -std=c++1y -mfpmath=sse -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -O3")

aux_source_directory(src/ SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})


find_package(OpenGL)
link_directories(/usr/local/lib/)
#link_directories(/Users/dokipen/code/openmp/runtime/exports/mac_32e/lib)
#set(CMAKE_EXE_LINKER_FLAGS "-framework iomp5")

#find_library(iomp5 NAMES libiomp5.dylib PATHS /Users/dokipen/code/openmp/runtime/exports/build/)

#MESSAGE(STATUS "Found iomp5 in: ${iomp5}")

#link_directories(/Users/dokipen/code/libcxx/build/lib)

target_link_libraries(${PROJECT_NAME} SDL2 GLEW ${OPENGL_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} SDL2 GLEW ${IOMP5_LIBRARIES} ${OPENGL_LIBRARIES})

