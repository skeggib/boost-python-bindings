#include "bindings_discovery.hpp"

#include <boost/python.hpp>

#include <iostream>
#include <string>
#include <vector>

std::vector<std::function<void()>> binders;

void register_binder(std::function<void()> binder)
{
    binders.push_back(binder);
}

struct my_exception : std::exception
{
    std::string reason;
    my_exception(const std::string &reason) : reason(reason) {}
    const char *what() const throw() { return reason.c_str(); }
};

void translate(my_exception const& e)
{
    // Use the Python 'C' API to set up an exception object
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

void bind_all()
{
    boost::python::register_exception_translator<my_exception>(&translate);
    if (binders.size() == 0)
    {
        static const std::string error = std::string("No binders where registered, make sure to call "
            "REGISTER_BINDER for each binder function and to link ") +
            std::string(__FILE__) +
            std::string(" before any file that registers a binder");
        std::cerr << error << std::endl;
        throw std::runtime_error(error);
    }
    for (auto binder : binders)
    {
        binder();
    }
}
