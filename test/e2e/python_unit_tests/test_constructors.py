import libE2EBindings as bindings

def test_default_constructor_is_bound():
    instance = bindings.DefaultedConstructors()
    assert instance is not None

def test_copy_constructor_is_bound():
    instance = bindings.DefaultedConstructors()
    instance.value = 'test'
    copy = bindings.DefaultedConstructors(instance)
    assert copy.value == instance.value

# TODO: existing value constructor

# TODO: non-existing default constructor
# TODO: non-existing copy constructor
# TODO: non-existing value constructor
