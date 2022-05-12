import libE2EBindings as bindings

def test_classes_are_bound():
    """Constructing a C++ class should not throw if it is bound"""
    world = bindings.World()
    assert world is not None

def test_methods_are_bound():
    world = bindings.World()
    world.set('Hello world!')
    assert world.greet() == 'Hello world!'

# TODO: public attributes
# TODO: protected attributes
# TODO: private attributes

# TODO: existing default constructor
# TODO: non-existing default constructor
# TODO: existing copy constructor
# TODO: non-existing copy constructor
# TODO: existing value constructor
# TODO: non-existing value constructor
