cmake_minimum_required(VERSION 3.4.1)
project(nmath2)

if ("${TARGET}" STREQUAL "macOS")
	if ( POLICY CMP0025 )
		cmake_policy(SET CMP0025 NEW)
	endif()
endif()
set(CMAKE_CXX_STANDARD 11)

option(LIBTYPE SHARED)
option(ADDTESTS off)

include_directories(${PROJECT_SOURCE_DIR}/include )
set( NMATH_SRC
    ${PROJECT_SOURCE_DIR}/src/nlablexer.cpp
    ${PROJECT_SOURCE_DIR}/src/internal_common.cpp
    ${PROJECT_SOURCE_DIR}/src/utf8.cpp
    ${PROJECT_SOURCE_DIR}/src/fraction.cpp
   )

if ("${TARGET}" STREQUAL "Android")
	add_definitions(-D_TARGET_HOST_ANDROID)
	add_library(nmath2 ${LIBTYPE} ${NMATH_SRC})
	find_library(log-lib log)
	target_link_libraries(nmath2 ${log-lib})
elseif ("${TARGET}" STREQUAL "macOS")
	add_definitions(-D_PCDEBUG)
	add_library(nmath2 ${LIBTYPE} ${NMATH_SRC})
	target_link_libraries(nmath2 pthread)
elseif ("${TARGET}" STREQUAL "Linux")
	add_library(nmath2 ${LIBTYPE} ${NMATH_SRC})
	target_link_libraries(nmath2 pthread)
elseif ("${TARGET}" STREQUAL "Windows")
	# Windows
	add_library(nmath2 ${LIBTYPE} ${NMATH_SRC})
	target_link_libraries(nmath2 pthread)
else()
	message("Invalid target!!!")
endif()

# Build test
if( DEFINED ${ADDTESTS} )
	include_directories(
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/test
	)
	set( TEST_SRC
		${PROJECT_SOURCE_DIR}/test/logging.cpp
		${PROJECT_SOURCE_DIR}/test/testParser.cpp
	)
	add_executable(testParser ${TEST_SRC})
	target_link_libraries(testParser nmath2)

	include_directories(
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/test
	)
	set( TESTF_SRC
		${PROJECT_SOURCE_DIR}/test/logging.cpp
		${PROJECT_SOURCE_DIR}/test/testFunction.cpp
	)
	add_executable(testFunction ${TESTF_SRC})
	target_link_libraries(testFunction nmath2)
endif()