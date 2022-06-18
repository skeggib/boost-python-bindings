#include "lazy.hpp"

#include <clang-c/Index.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class FunctionDecl
{
private:
    lazy<std::string> name_;

public:
    FunctionDecl(CXCursor cursor)
        : name_([cursor]()
                { return clang_getCString(clang_getCursorSpelling(cursor)); })
    {
    }
    const std::string &name() const { return name_; }
};

class ConstructorDecl
{
private:
    lazy<bool> is_copy_;

public:
    ConstructorDecl(CXCursor cursor)
        : is_copy_([cursor]()
                   { return clang_CXXConstructor_isCopyConstructor(cursor); })
    {
    }

    bool is_copy() const { return is_copy_; }
};

class MethodDecl
{
private:
    lazy<std::string> name_;

public:
    MethodDecl(CXCursor cursor)
        : name_([cursor]()
                { return clang_getCString(clang_getCursorSpelling(cursor)); })
    {
    }
    const std::string &name() const { return name_; }
};

enum AccessSpecifier
{
    Public,
    Protected,
    Private,
    Invalid
};
AccessSpecifier convert_clang_access_specifier(CX_CXXAccessSpecifier access_specifier)
{
    switch (access_specifier)
    {
    case CX_CXXInvalidAccessSpecifier:
        return AccessSpecifier::Invalid;
    case CX_CXXPublic:
        return AccessSpecifier::Public;
    case CX_CXXProtected:
        return AccessSpecifier::Protected;
    case CX_CXXPrivate:
        return AccessSpecifier::Private;
    default:
        return AccessSpecifier::Invalid;
    }
}

class FieldDecl
{
private:
    lazy<std::string> name_;
    lazy<AccessSpecifier> access_specifier_;

public:
    FieldDecl(CXCursor cursor)
        : name_([cursor]()
                { return clang_getCString(clang_getCursorSpelling(cursor)); }),
          access_specifier_([cursor]()
                            { return convert_clang_access_specifier(clang_getCXXAccessSpecifier(cursor)); })
    {
    }
    const std::string &name() const { return name_; }
    AccessSpecifier access_specifier() const { return access_specifier_; }
};

class ClassDecl
{
private:
    lazy<std::string> name_;
    lazy<std::vector<MethodDecl>> methods_;
    lazy<std::vector<FieldDecl>> fields_;
    lazy<std::vector<ConstructorDecl>> explicit_constructors_;
    lazy<std::vector<std::reference_wrapper<const ConstructorDecl>>> all_constructors_;

public:
    ClassDecl(CXCursor cursor)
        : name_([cursor]()
                { return clang_getCString(clang_getCursorSpelling(cursor)); }),
          methods_([cursor]()
                   {
                    std::vector<MethodDecl> methods;
                    clang_visitChildren(
                        cursor,
                        [](CXCursor cursor, CXCursor parent, CXClientData client_data){
                            std::vector<MethodDecl> *methods = static_cast<std::vector<MethodDecl> *>(client_data);
                            if (clang_getCursorKind(cursor) == CXCursor_CXXMethod)
                            {
                                methods->push_back(MethodDecl(cursor));
                            }
                            return CXChildVisit_Continue;
                        },
                        &methods);
                    return methods; }),
          fields_([cursor]()
                  {
                    std::vector<FieldDecl> fields;
                    clang_visitChildren(
                        cursor,
                        [](CXCursor cursor, CXCursor parent, CXClientData client_data){
                            std::vector<FieldDecl> *fields = static_cast<std::vector<FieldDecl> *>(client_data);
                            if (clang_getCursorKind(cursor) == CXCursor_FieldDecl)
                            {
                                fields->push_back(FieldDecl(cursor));
                            }
                            return CXChildVisit_Continue;
                        },
                        &fields);
                    return fields; }),
          explicit_constructors_([cursor]()
                                 {
                    std::vector<ConstructorDecl> constructors;
                    clang_visitChildren(
                        cursor,
                        [](CXCursor cursor, CXCursor parent, CXClientData client_data){
                            std::vector<ConstructorDecl> *constructors = static_cast<std::vector<ConstructorDecl> *>(client_data);
                            if (clang_getCursorKind(cursor) == CXCursor_Constructor)
                            {
                                constructors->push_back(ConstructorDecl(cursor));
                            }
                            return CXChildVisit_Continue;
                        },
                        &constructors);
                    return constructors; })
    {
    }
    const std::string &name() const { return name_; }
    const std::vector<MethodDecl> &methods() const { return methods_; }
    const std::vector<FieldDecl> &fields() const { return fields_; }
    const std::vector<ConstructorDecl> &explicit_constructors() const { return explicit_constructors_; }
    const std::vector<std::reference_wrapper<const ConstructorDecl>> &all_constructors() const
    {
        return all_constructors_.value_or([this]()
                                          {
        std::vector<std::reference_wrapper<const ConstructorDecl>> result(explicit_constructors().begin(), explicit_constructors().end());
        // if (std::find_if(explicit_constructors.begin(), explicit_constructors.end(), [](const ConstructorDecl &constructor){ constructor.is_default(); }))
        // if (explicit_constructors.size() == 0)
        // {
        //     result.push_back(ConstructorDecl::Default);
        //     result.push_back(ConstructorDecl::Copy);
        // }
        return result; });
    }

    bool has_copy_constructor() const
    {
        const auto &constructors = all_constructors();
        return std::find_if(constructors.begin(), constructors.end(), [](const ConstructorDecl &constructor)
                            { return constructor.is_copy(); }) != constructors.end();
    }
};

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
            data->functions.push_back(FunctionDecl(cursor));
            break;
        case CXCursor_ClassDecl:
        case CXCursor_StructDecl:
            data->classes.push_back(ClassDecl(cursor));
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
        output_file_stream << "    def(\"" << function.name() << "\", " << function.name() << ");\n";
    }
    for (auto &class_ : data.classes)
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