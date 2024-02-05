cmake_minimum_required(VERSION 3.4.1)
project(NautilusMath)

if ("${TARGET}" STREQUAL "macOS")
	if ( POLICY CMP0025 )
		cmake_policy(SET CMP0025 NEW)
	endif()
endif()

set_property(TARGET PROPERTY CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NMATH_CXX_FLAGS}")
message(${PROJECT_SOURCE_DIR})

option(LIBTYPE SHARED)
option(ADDTESTS off)

include_directories(${PROJECT_SOURCE_DIR}/include )

set( NMATH_SRC
    ${PROJECT_SOURCE_DIR}/src/nlablexer.cpp
    ${PROJECT_SOURCE_DIR}/src/internal_common.cpp
    ${PROJECT_SOURCE_DIR}/src/utf8.cpp
    ${PROJECT_SOURCE_DIR}/src/fraction.cpp
	${PROJECT_SOURCE_DIR}/src/StackUtil.cpp
   )

# How about iOS?
if ("${TARGET}" STREQUAL "Android")
	add_definitions(-D_TARGET_HOST_ANDROID)
	add_library(nmath2 ${LIBTYPE} ${NMATH_SRC})
	find_library(log-lib log)
	target_link_libraries(nmath2 ${log-lib} pthread)
elseif ("${TARGET}" STREQUAL "macOS")
	add_definitions(-D_PCDEBUG)
	add_library(nmath2 ${LIBTYPE} ${NMATH_SRC})
	target_link_libraries(nmath2 pthread)
elseif ("${TARGET}" STREQUAL "Linux")
	add_definitions(-D_NO_THREAD)
	add_library(nmath2 ${LIBTYPE} ${NMATH_SRC})
	target_link_libraries(nmath2 pthread)
elseif ("${TARGET}" STREQUAL "Windows")
	add_definitions(-D_NO_THREAD)
	add_library(nmath2 ${LIBTYPE} ${NMATH_SRC})
else()
	message("Invalid target!!!")
endif()

# Build test
if( DEFINED ADDTESTS )

	include_directories(
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/test
	)
	set( TESTF_SRC
		${PROJECT_SOURCE_DIR}/src/logging.cpp
		${PROJECT_SOURCE_DIR}/test/testFunction.cpp
	)
	add_executable(testFunction ${TESTF_SRC})
	target_link_libraries(testFunction nmath2)

	if ("${TARGET}" STREQUAL "Windows")
		set(GLEW_DIR ${PROJECT_SOURCE_DIR}/../glew-2.2.0-win32/glew-2.2.0/lib/Release/x64)
		set(GLFW_DIR ${PROJECT_SOURCE_DIR}/../glfw)
		include_directories(
			${PROJECT_SOURCE_DIR}/include
			${PROJECT_SOURCE_DIR}/test
			${PROJECT_SOURCE_DIR}/../nautilusnet/glm
			${PROJECT_SOURCE_DIR}/../glew-2.2.0-win32/glew-2.2.0/include
			${GLFW_DIR}/include
		)
		set( TESTGL_SRC
			${PROJECT_SOURCE_DIR}/test/testglm.cpp
		)
		add_executable(testGL ${TESTGL_SRC})
		# add_library(glew STATIC IMPORTED)
		# set_property(TARGET glew PROPERTY IMPORTED_IMPLIB "D:\\tools\\glew-2.2.0-win32\\glew-2.2.0\\lib\\Release\\x64\\glew32.lib")
		target_link_libraries(testGL opengl32 glu32 ${GLFW_DIR}/libvc2022/glfw3.lib ${GLEW_DIR}/glew32.lib nmath2)
	else()
		include_directories(
			${PROJECT_SOURCE_DIR}/include
			${PROJECT_SOURCE_DIR}/test
			${PROJECT_SOURCE_DIR}/../../../programs/glm-0.9.9.8/glm
		)
		set( TESTGL_SRC
			${PROJECT_SOURCE_DIR}/test/testglm.cpp
		)
		add_executable(testGL ${TESTGL_SRC})
		target_link_libraries(testGL OpenGL GL glfw GLEW nmath2)
	endif()
endif()
