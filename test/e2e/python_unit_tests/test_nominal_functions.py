import libE2EBindings as bindings

def test_functions_are_bound():
    """Calling a C++ function should not throw if it is bound"""
    bindings.doNothing()

def test_string_return_value_can_be_used():
    """A C++ std::string return value can be seamlessly compared with a Python str"""
    assert bindings.helloWorld() == 'Hello world'

def test_bound_function_can_take_parameters():
    assert bindings.add(1, 2) == 3
