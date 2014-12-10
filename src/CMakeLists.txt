project(FluidSim)
cmake_minimum_required(VERSION 2.8)

message("system: ${CMAKE_SYSTEM_PREFIX_PATH}")
#option(INTEL_CC "Use the Intel compiler." OFF)

#if (INTEL_CC)
#  find_program(CMAKE_C_COMPILER NAMES icc)
#  find_program(CMAKE_CXX_COMPILER NAMES icpc)
#  find_program(CMAKE_AR NAMES xiar)
#  find_program(CMAKE_LINKER NAMES xild)

#  if (CMAKE_C_COMPILER MATCHES CMAKE_C_COMPILER-NOTFOUND OR
#      CMAKE_CXX_COMPILER MATCHES CMAKE_CXX_COMPILER-NOTFOUND OR
#      CMAKE_AR MATCHES CMAKE_AR-NOTFOUND OR
#      CMAKE_LINKER MATCHES CMAKE_LINKER-NOTFOUND)
#    message(FATAL_ERROR "Cannot find Intel compiler.  You may need to run `. /opt/intel/bin/compilervars.sh intel64'")
#  endif ()
#endif (INTEL_CC)

#SET(CMAKE_C_COMPILER /opt/intel/composer_xe_2013_sp1.2.139/bin/intel64/icc)
#SET(CMAKE_CXX_COMPILER /opt/intel/composer_xe_2013_sp1.2.139/bin/intel64/icc)
#set(CMAKE_CXX_COMPILER /usr/local/bin/g++)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include")
set(CMAKE_C_COMPILER /Users/dokipen/code/llvm/build/Release/bin/clang2)
set(CMAKE_CXX_COMPILER /Users/dokipen/code/llvm/build/Release/bin/clang2++)
#set(CMAKE_C_COMPILER /usr/local/bin/gcc)
#set(CMAKE_CXX_COMPILER /usr/local/bin/g++)
#set(CMAKE_CXX_COMPILER /usr/local/bin/g++)



#set(CMAKE_CXX_COMPILER clang2++)
#include_directories(~/code/openmp/runtime/build/)
#include_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk/usr/include)
include_directories(/Users/dokipen/code/llvm/build/include/c++/v1)
include_directories(/Users/dokipen/code/openmp/runtime/build)

#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -std=c++1y -pedantic -mfpmath=sse -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -O3 -L /Users/dokipen/code/openmp/runtime/build/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -fopenmp -lstdc++ -stdlib=libc++ -std=c++1y -pedantic -mfpmath=sse -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -O3 -L/Users/dokipen/code/openmp/runtime/build/")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -lstdc++ -stdlib=libc++  -std=c++1y -pedantic -O3 -L /Users/dokipen/code/openmp/runtime/build/")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -march=native -O3 -std=c++1y -fopenmp")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -O0 -std=c++11")

#add_subdirectory(SDL2-2.0.3)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

find_package(OpenGL)
link_directories(/usr/local/lib/)
link_directories(/Users/dokipen/code/openmp/runtime/exports/mac_32e/lib.thin)

#link_directories(/Users/dokipen/code/libcxx/build/lib)

target_link_libraries (${PROJECT_NAME} SDL2 GLEW ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} )
