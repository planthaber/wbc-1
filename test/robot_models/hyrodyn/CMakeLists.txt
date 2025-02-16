find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories(${PROJECT_SOURCE_DIR}/src)

pkg_search_module(base-types REQUIRED base-types)
pkg_search_module(qpOASES REQUIRED qpOASES)
pkg_search_module(orocos-kdl REQUIRED orocos-kdl)
pkg_search_module(kdl_parser REQUIRED kdl_parser)
pkg_search_module (hyrodyn REQUIRED hyrodyn)
FIND_PACKAGE (RBDL COMPONENTS URDFReader REQUIRED)

include_directories(${base-types_INCLUDE_DIRS}
                    ${qpOASES_INCLUDE_DIRS}
                    ${orocos-kdl_INCLUDE_DIRS}
                    ${kdl_parser_INCLUDE_DIRS}
                    ${RBDL_INCLUDE_DIR}
                    ${hyrodyn_INCLUDE_DIR})
link_directories(${base-types_LIBRARY_DIRS}
                 ${qpOASES_LIBRARY_DIRS}
                 ${orocos-kdl_LIBRARY_DIRS}
                 ${kdl_parser_LIBRARY_DIRS}
                 ${RBDL_LIBRARY_DIRS}
                 ${hyrodyn_LIBRARY_DIRS})

add_executable(test_robot_model_kdl_vs_rbdl test_robot_model_kdl_vs_rbdl.cpp ../../suite.cpp)
target_link_libraries(test_robot_model_kdl_vs_rbdl
                      wbc-robot_models-kdl
                      wbc-tools
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      ${RBDL_LIBRARY}
                      ${RBDL_URDFReader_LIBRARY})

add_executable(test_robot_model_hyrodyn test_robot_model_hyrodyn.cpp ../../suite.cpp)
target_link_libraries(test_robot_model_hyrodyn
                      wbc-robot_models-hyrodyn
                      wbc-tools
                      wbc-robot_models-kdl
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

