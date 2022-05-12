import libE2EBindings as bindings

def test_structs_are_bound():
    """Constructing a C++ struct should not throw if it is bound"""
    date = bindings.Date()
    assert date is not None

def test_methods_are_bound():
    date = bindings.Date()
    assert date.toString() == '01/01/1970'

def test_public_attributes_are_bound():
    date = bindings.Date()
    # assign a value to a public attribute
    date.day = 2
    date.month = 3
    date.year = 2004
    # check that the value was taken into account in Python
    assert date.day == 2
    # check that the value was taken into account in C++
    assert date.toString() == '02/03/2004'

# TODO: protected attributes
# TODO: private attributes

# TODO: existing default constructor
# TODO: non-existing default constructor
# TODO: existing copy constructor
# TODO: non-existing copy constructor
# TODO: existing value constructor
# TODO: non-existing value constructor
