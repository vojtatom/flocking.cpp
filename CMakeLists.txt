# Min. CMake version.
cmake_minimum_required(VERSION 2.8.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_PREFIX_PATH "./")
set(GLEW_DIR "./")

# Project name.
set(NAME flocking)

# A name of the project.
project(${NAME})

find_package(GLUT)
find_package(GLEW)
find_package(OpenGL REQUIRED)

# Set general compiler flags.
if (WIN32)
	add_definitions(-D__WINDOWS__ -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)
endif (WIN32)

if (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenGL -framework GLUT")
endif (APPLE)


add_definitions(-Wall -pedantic -g)

# Flocking headers and cpps.
file(GLOB F_HDR "src/*.h")
file(GLOB F_SRC "src/*.cpp")

set(HDR ${F_HDR})
set(SRC ${F_SRC} main.cpp)

# Find includes in corresponding build directories.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

source_group("Header Files\\" FILES ${HDR})
source_group("Source Files\\" FILES ${SRC})

# Add all files to excutable.
add_executable(${NAME}
  ${HDR}
  ${SRC}
)

# Use freeglut on Windows.
if (WIN32)
	if (NOT GLUT_FOUND)
		if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(GLUT_LIB_PATH freeglut/lib/x64)
	else(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(GLUT_LIB_PATH freeglut/lib)
	endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
	find_library(GLUT_LIBRARIES
		NAMES freeglut
		PATHS
		${GLUT_LIB_PATH}
		DOC "The GLUT library"
	)
	find_path(GLUT_INCLUDE_DIR GL/glut.h
		freeglut/include
		DOC "The directory where GL/glut.h resides"
	)
	endif (NOT GLUT_FOUND)
endif (WIN32)

# Add include directories.
set(SGL_INCLUDE_DIR ./sgl)
include_directories (SYSTEM 
	${SGL_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_definitions(-DNDEBUG)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra")
	#set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

target_link_libraries (${NAME} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

# Set output directory.
set(BIN_DIR ${CMAKE_SOURCE_DIR})

# Properties.
set_target_properties(${NAME} PROPERTIES 
	DEBUG_OUTPUT_NAME ${NAME}_d 
	RELEASE_OUTPUT_NAME ${NAME}	
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}
)


