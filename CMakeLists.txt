cmake_minimum_required (VERSION 3.8)

project ("geoCFD")

set (CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include) # include json.hpp

find_package(CGAL) # for CGALgeocfd
if (CGAL_FOUND)
	include(${CGAL_USE_FILE})
	message(STATUS "CGAL found")
else()
	message(SEND_ERROR "this code requires the CGAL library")
	return()
endif()

add_executable (geoCFD "src/main.cpp" "src/JsonHandler.hpp" "src/Polyhedron.hpp" "src/JsonWriter.hpp" "src/Evaluation.h")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
