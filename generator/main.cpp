#include <clang-c/Index.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct FunctionDecl
{
    std::string name;
};
FunctionDecl parse_function_decl(CXCursor cursor)
{
    auto name = clang_getCString(clang_getCursorSpelling(cursor));
    std::cout << "[debug] parsing function: " << name << std::endl;
    return FunctionDecl{name};
}

struct ConstructorDecl
{
    CXCursor cursor;
    std::vector<std::string> parameter_types; // TODO: parse types

    bool is_copy() const
    {
        return clang_CXXConstructor_isCopyConstructor(cursor);
    }
};
CXChildVisitResult constructor_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    ConstructorDecl *constructor = static_cast<ConstructorDecl *>(client_data);
    auto cursorKind = clang_getCursorKind(cursor);
    switch (cursorKind)
    {
    case CXCursor_ParmDecl:
        constructor->parameter_types.push_back(clang_getCString(clang_getTypeSpelling(clang_getCursorType(cursor))));
        break;
    default:
        // do nothing
        break;
    }
    return CXChildVisit_Continue;
}
ConstructorDecl parse_constructor_decl(CXCursor cursor)
{
    ConstructorDecl constructor;
    constructor.cursor = cursor;
    clang_visitChildren(
        cursor,
        constructor_visitor,
        &constructor);
    return constructor;
}

struct MethodDecl
{
    std::string name;
};
MethodDecl parse_method_decl(CXCursor cursor)
{
    auto name = clang_getCString(clang_getCursorSpelling(cursor));
    std::cout << "[debug] parsing method: " << name << std::endl;
    return MethodDecl{name};
}

struct FieldDecl
{
    std::string name;
};
FieldDecl parse_field_decl(CXCursor cursor)
{
    auto name = clang_getCString(clang_getCursorSpelling(cursor));
    std::cout << "[debug] parsing field: " << name << std::endl;
    return FieldDecl{name};
}

struct ClassDecl
{
    std::string name;
    std::vector<MethodDecl> methods;
    std::vector<FieldDecl> fields;
    std::vector<ConstructorDecl> explicit_constructors;

    std::vector<std::reference_wrapper<const ConstructorDecl>> all_constructors() const
    {
        std::vector<std::reference_wrapper<const ConstructorDecl>> result(explicit_constructors.begin(), explicit_constructors.end());
        // if (std::find_if(explicit_constructors.begin(), explicit_constructors.end(), [](const ConstructorDecl &constructor){ constructor.is_default(); }))
        // if (explicit_constructors.size() == 0)
        // {
        //     result.push_back(ConstructorDecl::Default);
        //     result.push_back(ConstructorDecl::Copy);
        // }
        return result;
    }

    bool has_copy_constructor() const
    {
        const auto &constructors = all_constructors();
        return std::find_if(constructors.begin(), constructors.end(), [](const ConstructorDecl &constructor){ return constructor.is_copy(); }) != constructors.end();
    }
};
CXChildVisitResult class_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    ClassDecl *class_ = static_cast<ClassDecl *>(client_data);
    auto cursorKind = clang_getCursorKind(cursor);
    switch (cursorKind)
    {
    case CXCursor_Constructor:
        class_->explicit_constructors.push_back(parse_constructor_decl(cursor));
        break;
    case CXCursor_CXXMethod:
        class_->methods.push_back(parse_method_decl(cursor));
        break;
    case CXCursor_FieldDecl:
        if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic)
        {
            class_->fields.push_back(parse_field_decl(cursor));
        }
        break;
    default:
        // do nothing
        break;
    }
    return CXChildVisit_Continue;
}
ClassDecl parse_class_decl(CXCursor cursor)
{
    auto name = clang_getCString(clang_getCursorSpelling(cursor));
    std::cout << "[debug] parsing class: " << name << std::endl;
    ClassDecl class_{name};
    clang_visitChildren(
        cursor,
        class_visitor,
        &class_);
    return class_;
}

struct VisitorData
{
    VisitorData(const std::string &input_file_path)
        : input_file_path(input_file_path)
    {
    }
    std::string input_file_path;
    std::vector<FunctionDecl> functions;
    std::vector<ClassDecl> classes;
};

CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    VisitorData *data = static_cast<VisitorData *>(client_data);
    CXFile file;
    clang_getSpellingLocation(clang_getCursorLocation(cursor), &file, nullptr, nullptr, nullptr);
    if (clang_getCString(clang_getFileName(file)) == data->input_file_path)
    {
        std::string cursorSpelling(clang_getCString(clang_getCursorSpelling(cursor)));
        auto cursorKind = clang_getCursorKind(cursor);
        switch (cursorKind)
        {
        case CXCursor_FunctionDecl:
            data->functions.push_back(parse_function_decl(cursor));
            break;
        case CXCursor_ClassDecl:
        case CXCursor_StructDecl:
            data->classes.push_back(parse_class_decl(cursor));
            break;
        default:
            // do nothing
            break;
        }
    }
    return CXChildVisit_Continue;
}

int main(int argc, char **argv)
{
    std::string input_file_path(argv[1]);
    std::string output_file_path(argv[2]);
    std::cout << "[debug] input: " << input_file_path << std::endl;
    std::cout << "[debug] output: " << output_file_path << std::endl;

    std::cout << "[debug] creating clang index" << std::endl;
    CXIndex index = clang_createIndex(0, 0);
    std::cout << "[debug] parsing translation unit" << std::endl;
    CXTranslationUnit translation_unit;
    clang_parseTranslationUnit2(
        index,
        input_file_path.c_str(),
        nullptr, // no command-line args
        0,       // no command-line args
        nullptr, // no unsaved files
        0,       // no unsaved files
        CXTranslationUnit_None,
        &translation_unit);

    CXCursor cursor = clang_getTranslationUnitCursor(translation_unit);

    VisitorData data(input_file_path);
    clang_visitChildren(
        cursor,
        visitor,
        &data);

    std::cout
        << "[debug] disposing of translation unit" << std::endl;
    clang_disposeTranslationUnit(translation_unit);
    std::cout << "[debug] disposing of clang index" << std::endl;
    clang_disposeIndex(index);

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
    for (auto &function : data.functions)
    {
        output_file_stream << "    def(\"" << function.name << "\", " << function.name << ");\n";
    }
    for (auto &class_ : data.classes)
    {
        output_file_stream << "    class_<" << class_.name << ">(\"" << class_.name << "\")\n";
        if (class_.has_copy_constructor())
        {
            output_file_stream << "        .def(init<" << class_.name << " const &>())\n";
        }
        for (auto &method : class_.methods)
        {
            output_file_stream << "        .def(\"" << method.name << "\", &" << class_.name << "::" << method.name << ")\n";
        }
        for (auto &field : class_.fields)
        {
            output_file_stream << "        .def_readwrite(\"" << field.name << "\", &" << class_.name << "::" << field.name << ")\n";
        }
        output_file_stream << "    ;\n";
    }
    output_file_stream << "} // bind\n";
    output_file_stream << "} // namespace\n";
    output_file_stream << "REGISTER_BINDER(bind)\n";

    output_file_stream.close();

    return 0;
}