project(“pph” CXX)
cmake_minimum_required(VERSION 3.1)

if(APPLE)
  set(CMAKE_MACOSX_RPATH 1)
  set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif()

# This ensures that things like gnu++11 get passed correctly
set(CMAKE_CXX_STANDARD 11)

# We require a C++11 compliant compiler
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# where to find cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
set(BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/build-support")

find_package(ClangTools)
if ("$ENV{CMAKE_EXPORT_COMPILE_COMMANDS}" STREQUAL "1" OR CLANG_TIDY_FOUND)
  # Generate a Clang compile_commands.json "compilation database" file for use
  # with various development tools, such as Vim's YouCompleteMe plugin.
  # See http://clang.llvm.org/docs/JSONCompilationDatabase.html
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()

find_package(InferTools)
if ("$ENV{CMAKE_EXPORT_COMPILE_COMMANDS}" STREQUAL "1" OR INFER_FOUND)
  # Generate a Clang compile_commands.json "compilation database" file for use
  # with various development tools, such as Vim's YouCompleteMe plugin.
  # See http://clang.llvm.org/docs/JSONCompilationDatabase.html
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()


if(APPLE)
  set(CMAKE_MACOSX_RPATH 1)
endif()

# if no build build type is specified, default to debug builds
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

# Determine compiler version
include(CompilerInfo)

if ("${COMPILER_FAMILY}" STREQUAL "clang")
  # Using Clang with ccache causes a bunch of spurious warnings that are
  # purportedly fixed in the next version of ccache. See the following for details:
  #
  #   http://petereisentraut.blogspot.com/2011/05/ccache-and-clang.html
  #   http://petereisentraut.blogspot.com/2011/09/ccache-and-clang-part-2.html
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CLANG_OPTIONS}")
endif()


############################################################
# "make lint" target
############################################################
if (UNIX)
  find_program(CPPLINT_BIN NAMES cpplint cpplint.py HINTS ${BUILD_SUPPORT_DIR})
  message(STATUS "Found cpplint executable at ${CPPLINT_BIN}")
  # Full lint
  add_custom_target(lint ${CPPLINT_BIN}
  --verbose=2
  --linelength=90
  --filter=-whitespace/comments,-readability/todo,-build/header_guard,-runtime/references,-readability/check,-build/c++11,-build/include_order
    `find ${CMAKE_CURRENT_SOURCE_DIR}/ -name \\*.cpp -or -name \\*.h | sed -e '/pph\\/pph_/g'`)
endif (UNIX)


############################################################
# "make format" and "make check-format" targets
############################################################

if (${CLANG_FORMAT_FOUND})
  # runs clang format and updates files in place.
  add_custom_target(format ${BUILD_SUPPORT_DIR}/run-clang-format.sh ${CMAKE_CURRENT_SOURCE_DIR} ${CLANG_FORMAT_BIN} 1
  `find ${CMAKE_CURRENT_SOURCE_DIR}/ -name \\*.cpp -or -name \\*.h | sed -e '/_generated/g'`)

  # runs clang format and exits with a non-zero exit code if any files need to be reformatted
  add_custom_target(check-format ${BUILD_SUPPORT_DIR}/run-clang-format.sh ${CMAKE_CURRENT_SOURCE_DIR} ${CLANG_FORMAT_BIN} 0
  `find ${CMAKE_CURRENT_SOURCE_DIR}/ -name \\*.cpp -or -name \\*.h | sed -e '/_generated/g'`)
endif()


############################################################
# "make clang-tidy" and "make check-clang-tidy" targets
############################################################

if (${CLANG_TIDY_FOUND})
  # runs clang-tidy and attempts to fix any warning automatically
  add_custom_target(clang-tidy ${BUILD_SUPPORT_DIR}/run-clang-tidy.sh ${CLANG_TIDY_BIN} ${CMAKE_BINARY_DIR}/compile_commands.json 1
  `find ${CMAKE_CURRENT_SOURCE_DIR}/ -name \\*.cpp | sed -e '/_types/g' | sed -e '/_constants/g'`)
  # runs clang-tidy and exits with a non-zero exit code if any errors are found.
  add_custom_target(check-clang-tidy ${BUILD_SUPPORT_DIR}/run-clang-tidy.sh ${CLANG_TIDY_BIN} ${CMAKE_BINARY_DIR}/compile_commands.json
  0 `find ${CMAKE_CURRENT_SOURCE_DIR}/ -name \\*.cpp |grep -v -F -f ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy-ignore`)
endif()


############################################################
# "make infer" target
############################################################

if (${INFER_FOUND})
  # runs infer capture
  add_custom_target(infer ${BUILD_SUPPORT_DIR}/run-infer.sh ${INFER_BIN} ${CMAKE_BINARY_DIR}/compile_commands.json 1)
  # runs infer analyze
  add_custom_target(infer-analyze ${BUILD_SUPPORT_DIR}/run-infer.sh ${INFER_BIN} ${CMAKE_BINARY_DIR}/compile_commands.json 2)
  # runs infer report
  add_custom_target(infer-report ${BUILD_SUPPORT_DIR}/run-infer.sh ${INFER_BIN} ${CMAKE_BINARY_DIR}/compile_commands.json 3)
endif()

# version string
set(PPH_VERSION_MAJOR "1")
set(PPH_VERSION_MINOR "0")
set(PPH_VERSION_PATCH "0")
set(PPH_VERSION_EXTRA "dev")
set(PPH_VERSION_RAW "${PPH_VERSION_MAJOR}.${PPH_VERSION_MINOR}.${PPH_VERSION_PATCH}${PPH_VERSION_EXTRA}")
string(TIMESTAMP PPH_BUILD_DATE "%Y%m%d")

if($ENV{BUILD_NUMBER})
  set(PPH_BUILD_NUMBER "$ENV{BUILD_NUMBER}")
else()
  set(PPH_BUILD_NUMBER "dev")
endif()
set(PPH_VERSION "${PPH_VERSION_RAW}-${PPH_BUILD_NUMBER}")

execute_process(
  # NOTE: Uncomment following line after checking into GIT
  COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
  # TODO: Delete following line after checking into GIT
  #COMMAND echo "ccccccc"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE PPH_GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
file(WRITE ${CMAKE_BINARY_DIR}/PPH_GIT_HASH.txt "${PPH_GIT_HASH}\n" )
file(STRINGS ${CMAKE_BINARY_DIR}/PPH_GIT_HASH.txt PPH_GIT_HASH)
set(CPACK_PACKAGE_VERSION "${PPH_VERSION_RAW}-${PPH_BUILD_DATE}-${PPH_GIT_HASH}")

# list of sources files
set(PPH_SRC
 ${CMAKE_SOURCE_DIR}/pph.cpp
 ${CMAKE_SOURCE_DIR}/SpookyV2.cpp
 ${CMAKE_SOURCE_DIR}/GcdBinary.cpp
 ${CMAKE_SOURCE_DIR}/bitScanForward.cpp
 ${CMAKE_SOURCE_DIR}/bitScanReverse.cpp
)

# list of header files
set(PPH_INC
 ${CMAKE_SOURCE_DIR}/pph.h
 ${CMAKE_SOURCE_DIR}/XorShift1024Star.h
 ${CMAKE_SOURCE_DIR}/fnv64a_hash.h
 ${CMAKE_SOURCE_DIR}/SpookyV2.h
 ${CMAKE_SOURCE_DIR}/bitScanForward.h
 ${CMAKE_SOURCE_DIR}/bitScanReverse.h
 ${CMAKE_SOURCE_DIR}/GcdBinary.h
 ${CMAKE_SOURCE_DIR}/PrimeNumber.h
 ${CMAKE_SOURCE_DIR}/PowerOfTwo.h
 ${CMAKE_SOURCE_DIR}/StringUtil.h
 ${CMAKE_BINARY_DIR}/pphrelease.h
)

find_package(Boost REQUIRED COMPONENTS thread regex program_options filesystem system date_time)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})
message(STATUS "Boost include dir: " ${Boost_INCLUDE_DIRS})
message(STATUS "Boost libraries: " ${Boost_LIBRARIES})

add_executable(pph ${PPH_SRC} ${PPH_INC})
target_link_libraries(pph ${Boost_LIBRARIES})
target_include_directories(pph PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS})

# generate header with version number
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/release.h"
  "${CMAKE_BINARY_DIR}/pphrelease.h"
  @ONLY
  )

list(APPEND ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/PPH_GIT_HASH.txt)
list(APPEND ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/pphrelease.h)

# required to force regen of PPH_GIT_HASH.txt, pphrelease.h
add_custom_target(rerun_cmake ALL
   COMMAND cmake .
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   )

add_dependencies(pph
   rerun_cmake
   )
