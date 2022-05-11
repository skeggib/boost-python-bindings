# generate bindings
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bindings)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bindings/nominal.cpp
    COMMAND generator ${CMAKE_CURRENT_SOURCE_DIR}/include/nominal.hpp ${CMAKE_CURRENT_BINARY_DIR}/bindings/nominal.cpp
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/include/nominal.hpp
    IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/include/nominal.hpp
    DEPENDS generator
)

# build bindings
find_package(Boost COMPONENTS python REQUIRED)
find_package (Python3 COMPONENTS Development)

include_directories(${Python3_INCLUDE_DIRS})
include_directories(include)
add_library(
    E2EBindings SHARED
    ${CMAKE_CURRENT_BINARY_DIR}/bindings/nominal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nominal.cpp
)
target_link_libraries(
    E2EBindings
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
)

# run tests

add_test(
    NAME e2e/classes
    COMMAND pytest test_classes.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python_unit_tests
)
set_tests_properties(e2e/classes PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}")

add_test(
    NAME e2e/functions
    COMMAND pytest test_functions.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python_unit_tests
)
set_tests_properties(e2e/functions PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}")
