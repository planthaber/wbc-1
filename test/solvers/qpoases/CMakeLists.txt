find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories(${PROJECT_SOURCE_DIR}/src)

pkg_search_module(base-types REQUIRED base-types)
pkg_search_module(qpOASES REQUIRED qpOASES)
include_directories(${base-types_INCLUDE_DIRS}
                    ${qpOASES_INCLUDE_DIRS})
link_directories(${base-types_LIBRARY_DIRS}
                 ${qpOASES_LIBRARY_DIRS})

add_executable(test_qpoases_solver test_qpoases_solver.cpp ../../suite.cpp)
target_link_libraries(test_qpoases_solver
                      wbc-solvers-qpoases
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
