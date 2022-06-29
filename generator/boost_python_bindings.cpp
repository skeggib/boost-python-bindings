#include "boost_python_bindings.hpp"

#include <algorithm>
#include <filesystem>

std::ostream &operator<<(std::ostream &stream, const FunctionExposer &function_exposer)
{
    stream << "    def(\"" << function_exposer.name() << "\", " << function_exposer.name() << ");";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const ParameterExposer &parameter_exposer)
{
    stream << parameter_exposer.type();
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const ConstructorExposer &constructor_exposer)
{
    stream << "        .def(init<";
    for (size_t i = 0; i < constructor_exposer.parameters().size(); ++i)
    {
        if (i > 0)
            stream << ", ";
        stream << constructor_exposer.parameters()[i];
    }
    stream << ">())";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const MethodExposer &method_exposer)
{
    stream << "        .def(\"" << method_exposer.name() << "\", &" << method_exposer.fully_qualified_name() << ")";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const FieldExposer &field_exposer)
{
    stream << "        .def_readwrite(\"" << field_exposer.name() << "\", &" << field_exposer.fully_qualified_name() << ")";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const ClassExposer &class_exposer)
{
    stream << "    class_<" << class_exposer.name() << ">(\"" << class_exposer.name() << "\")\n";
    for (auto &constructor_exposer : class_exposer.constructors())
        stream << constructor_exposer << "\n";
    for (auto &method_exposer : class_exposer.methods())
        stream << method_exposer << "\n";
    for (auto &field_exposer : class_exposer.fields())
        stream << field_exposer << "\n";
    stream << "    ;";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, TranslationUnitExposer translation_unit_exposer)
{
    stream << "#include \"" << std::filesystem::path(translation_unit_exposer.input_file_path()).filename().c_str() << "\"\n";
    stream << "#include \"bindings_discovery.hpp\"\n";
    stream << "#include <boost/python.hpp>\n";
    stream << "namespace {\n";
    stream << "void bind()\n";
    stream << "{\n";
    stream << "    using namespace boost::python;\n";
    for (auto &function_exposer : translation_unit_exposer.functions())
        stream << function_exposer << "\n";
    for (auto &class_exposer : translation_unit_exposer.classes())
        stream << class_exposer << "\n";
    stream << "} // bind\n";
    stream << "} // namespace\n";
    stream << "REGISTER_BINDER(bind)\n";

    return stream;
}

TranslationUnitExposer expose(const TranslationUnit &translation_unit)
{
    std::vector<FunctionExposer> functions;
    std::vector<ClassExposer> classes;
    std::transform(
        translation_unit.functions().begin(),
        translation_unit.functions().end(),
        std::back_inserter(functions),
        [](const FunctionDecl &function)
        { return FunctionExposer(function.name()); });
    std::transform(
        translation_unit.classes().begin(),
        translation_unit.classes().end(),
        std::back_inserter(classes),
        [](const ClassDecl &class_)
        {
            std::vector<ParameterExposer> parameters;
            parameters.push_back(ParameterExposer(class_.name() + "  const &"));
            std::vector<ConstructorExposer> constructors;
            constructors.push_back(ConstructorExposer(parameters)); // TODO: wip

            std::vector<MethodExposer> methods;
            for (auto &method : class_.methods())
                methods.push_back(MethodExposer(method.name(), class_.name() + "::" + method.name()));

            std::vector<FieldExposer> fields;
            for (auto &field : class_.fields())
                if (field.access_specifier() != AccessSpecifier::Private)
                    fields.push_back(FieldExposer(field.name(), class_.name() + "::" + field.name()));

            return ClassExposer(class_.name(), constructors, methods, fields);
        });
    return TranslationUnitExposer(translation_unit.path(), functions, classes);
}

void generate(const TranslationUnitExposer &exposer, std::ostream &stream)
{
    stream << exposer;
}
