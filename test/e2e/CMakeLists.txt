# generate bindings
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bindings)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bindings/nominal.cpp
    COMMAND generator ${CMAKE_CURRENT_SOURCE_DIR}/include/nominal.hpp ${CMAKE_CURRENT_BINARY_DIR}/bindings/nominal.cpp
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/include/nominal.hpp
    IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/include/nominal.hpp
    DEPENDS generator
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bindings/constructors.cpp
    COMMAND generator ${CMAKE_CURRENT_SOURCE_DIR}/include/constructors.hpp ${CMAKE_CURRENT_BINARY_DIR}/bindings/constructors.cpp
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/include/constructors.hpp
    IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/include/constructors.hpp
    DEPENDS generator
)
set(BINDINGS_LIB_NAME E2EBindings)
configure_file(
    ${CMAKE_SOURCE_DIR}/generator/module_entrypoint.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/bindings/module_entrypoint.cpp
)

# build bindings
find_package(Boost COMPONENTS python REQUIRED)
find_package (Python3 COMPONENTS Development)

include_directories(${Python3_INCLUDE_DIRS})
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/generator)
add_library(
    ${BINDINGS_LIB_NAME} SHARED

    # bindings_discovery.cpp must be linked before bindings files for the
    # binders list to be initialized before any binder is registered
    ${CMAKE_SOURCE_DIR}/generator/bindings_discovery.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/bindings/module_entrypoint.cpp

    # bindings
    ${CMAKE_CURRENT_BINARY_DIR}/bindings/constructors.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/bindings/nominal.cpp

    # user code
    ${CMAKE_CURRENT_SOURCE_DIR}/src/constructors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nominal.cpp
)
target_link_libraries(
    ${BINDINGS_LIB_NAME}
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
)
set_property(TARGET ${BINDINGS_LIB_NAME} PROPERTY CXX_STANDARD 20)

# run tests

function(add_pytest test_name)
    add_test(
        NAME e2e/${test_name}
        COMMAND pytest test_${test_name}.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python_unit_tests
    )
    set_tests_properties(e2e/${test_name} PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

add_pytest(nominal_classes)
add_pytest(nominal_functions)
add_pytest(nominal_structs)
add_pytest(constructors)
