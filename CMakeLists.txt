cmake_minimum_required(VERSION 3.0)
project(calculator VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS python REQUIRED)
# include_directories(${Boost_INCLUDE_DIRS})
# link_directories(${Boost_LIBRARY_DIRS})

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
# link_directories(${PYTHON_LIBRARIES})

add_library(calculator_bindings SHARED
    calculator.cpp
    calculator_bindings.cpp
)
target_link_libraries(calculator_bindings
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
)
