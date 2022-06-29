#include "boost_python_bindings.hpp"
#include "clang_parser.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char **argv)
{
    std::string input_file_path(argv[1]);
    std::string output_file_path(argv[2]);
    std::cout << "[debug] input: " << input_file_path << std::endl;
    std::cout << "[debug] output: " << output_file_path << std::endl;

    auto translation_unit = parse(input_file_path);
    auto exposer = expose(translation_unit);

    std::ofstream stream;
    stream.open(output_file_path);
    generate(exposer, stream);
    stream.close();

    return 0;
}
