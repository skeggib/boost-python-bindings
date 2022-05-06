cmake_minimum_required(VERSION 3.12)

project(boost-python-bindings)
enable_testing()

set(CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}
)
find_package(LibClang)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBCLANG_CXXFLAGS} ")
set(CMAKE_CXX_)

add_subdirectory(generator)
add_subdirectory(test)