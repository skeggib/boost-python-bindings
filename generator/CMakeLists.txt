add_executable(
    generator
    boost_python_bindings.cpp
    clang_parser.cpp
    main.cpp
)
target_link_libraries(generator ${LIBCLANG_LIBRARIES})

set_property(TARGET generator PROPERTY CXX_STANDARD 20)