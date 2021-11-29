#include "calculator.hpp"

#include <boost/python.hpp>

BOOST_PYTHON_MODULE(libcalculator_bindings)
{
    using namespace boost::python;

    class_<Calculator>("Calculator")
        .def("add", &Calculator::add)
        .def("substract", &Calculator::substract)
        .def("multiply", &Calculator::multiply)
        .def("divide", &Calculator::divide);
}
