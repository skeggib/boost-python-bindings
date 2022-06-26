#include "clang_parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char **argv)
{
    std::string input_file_path(argv[1]);
    std::string output_file_path(argv[2]);
    std::cout << "[debug] input: " << input_file_path << std::endl;
    std::cout << "[debug] output: " << output_file_path << std::endl;

    auto ast = parse(input_file_path);

    std::ofstream output_file_stream;
    std::cout << "[debug] writing output file" << std::endl;
    output_file_stream.open(output_file_path);

    output_file_stream << "#include \"" << std::filesystem::path(input_file_path).filename().c_str() << "\"\n";
    output_file_stream << "#include \"bindings_discovery.hpp\"\n";
    output_file_stream << "#include <boost/python.hpp>\n";
    output_file_stream << "namespace {\n";
    output_file_stream << "void bind()\n";
    output_file_stream << "{\n";
    output_file_stream << "    using namespace boost::python;\n";
    for (auto &function : ast.functions())
    {
        output_file_stream << "    def(\"" << function.name() << "\", " << function.name() << ");\n";
    }
    for (auto &class_ : ast.classes())
    {
        output_file_stream << "    class_<" << class_.name() << ">(\"" << class_.name() << "\")\n";
        if (class_.has_copy_constructor())
        {
            output_file_stream << "        .def(init<" << class_.name() << " const &>())\n";
        }
        for (auto &method : class_.methods())
        {
            output_file_stream << "        .def(\"" << method.name() << "\", &" << class_.name() << "::" << method.name() << ")\n";
        }
        for (auto &field : class_.fields())
        {
            if (field.access_specifier() != AccessSpecifier::Private)
            {
                output_file_stream << "        .def_readwrite(\"" << field.name() << "\", &" << class_.name() << "::" << field.name() << ")\n";
            }
        }
        output_file_stream << "    ;\n";
    }
    output_file_stream << "} // bind\n";
    output_file_stream << "} // namespace\n";
    output_file_stream << "REGISTER_BINDER(bind)\n";

    output_file_stream.close();

    return 0;
}