cmake_minimum_required(VERSION 2.8.11)

find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)

set(PYTHON_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)

if (UNIX AND NOT APPLE)
    if (PYTHON_VERSION_MAJOR EQUAL 3)
        find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR} numpy${PYTHON_VERSION_MAJOR})
    else()
        find_package(Boost COMPONENTS python numpy)
    endif()
else()
    if (PYTHON_VERSION_MAJOR EQUAL 3)
        find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} numpy${PYTHON_VERSION_MAJOR})
    else()
        find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} numpy${PYTHON_VERSION_MAJOR})
    endif()
endif()

message(STATUS "PYTHON_VERSION_MAJOR.PYTHON_VERSION_MINOR = ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
message(STATUS "PYTHON_INSTALL_PATH = ${PYTHON_INSTALL_PATH}")

pkg_search_module(base-types REQUIRED base-types)
pkg_search_module(orocos-kdl REQUIRED orocos-kdl)
pkg_search_module(kdl_parser REQUIRED kdl_parser)
list(APPEND PKGCONFIG_REQUIRES qpOASES)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${base-types_INCLUDE_DIRS} ${orocos-kdl_INCLUDE_DIRS} ${qpOASES_INCLUDE_DIRS} ${kdl_parser_INCLUDE_DIRS})
link_directories(${base-types_LIBRARY_DIRS} ${orocos-kdl_LIBRARY_DIRS} ${qpOASES_LIBRARY_DIRS} ${kdl_parser_LIBRARY_DIRS})

PYTHON_ADD_MODULE(scenes scenes.cpp)

# Set up the libraries and header search paths for this target
target_link_libraries(scenes ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} wbc-scenes wbc-robot_models-kdl wbc-core wbc-solvers-hls wbc-solvers-qpoases ${qpOASES_LIBRARIES})
target_include_directories(scenes PRIVATE ${PYTHON_INCLUDE_DIRS} )

install(TARGETS scenes
        DESTINATION ${PYTHON_INSTALL_PATH}/wbc)
