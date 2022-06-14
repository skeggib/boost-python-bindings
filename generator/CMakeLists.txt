add_executable(
    generator
    main.cpp
)
target_link_libraries(generator ${LIBCLANG_LIBRARIES})

set_property(TARGET generator PROPERTY CXX_STANDARD 20)