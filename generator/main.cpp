#include "lazy.hpp"
#include "visitors.hpp"

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
    lazy_placeholder<std::vector<std::reference_wrapper<const ConstructorDecl>>> all_constructors_;

public:
    ClassDecl(CXCursor cursor)
        : name_(
              [cursor]()
              { return clang_getCString(clang_getCursorSpelling(cursor)); }),
          methods_(
              [cursor]()
              { return mapChildren<CXCursor_CXXMethod, MethodDecl>(cursor); }),
          fields_(
              [cursor]()
              { return mapChildren<CXCursor_FieldDecl, FieldDecl>(cursor); }),
          explicit_constructors_(
              [cursor]()
              { return mapChildren<CXCursor_Constructor, ConstructorDecl>(cursor); })
    {
    }
    const std::string &name() const { return name_; }
    const std::vector<MethodDecl> &methods() const { return methods_; }
    const std::vector<FieldDecl> &fields() const { return fields_; }
    const std::vector<ConstructorDecl> &explicit_constructors() const { return explicit_constructors_; }
    const std::vector<std::reference_wrapper<const ConstructorDecl>> &all_constructors() const
    {
        return all_constructors_.set_retriever(
            [this]()
            {
                return std::vector<std::reference_wrapper<const ConstructorDecl>>(
                    explicit_constructors().begin(), explicit_constructors().end());
            });
    }

    bool has_copy_constructor() const
    {
        auto constructors = all_constructors();
        return std::find_if(constructors.begin(), constructors.end(), [](const ConstructorDecl &constructor)
                            { return constructor.is_copy(); }) != constructors.end();
    }
};

class TranslationUnit
{
private:
    std::string input_file_path_;
    std::vector<FunctionDecl> functions_;
    std::vector<ClassDecl> classes_;

public:
    TranslationUnit(CXCursor cursor, std::string input_file_path)
        : input_file_path_(input_file_path)
    {
        clang_visitChildren(
            cursor,
            [](CXCursor cursor, CXCursor parent, CXClientData client_data)
            {
                auto data = static_cast<TranslationUnit *>(client_data);
                CXFile file;
                unsigned int line;
                unsigned int column;
                unsigned int offset;
                // TODO: this segfaults when used in lazy fields, find a way to prevent that
                clang_getSpellingLocation(clang_getCursorLocation(cursor), &file, &line, &column, nullptr);
                if (clang_getCString(clang_getFileName(file)) == data->input_file_path_)
                {
                    std::string cursorSpelling(clang_getCString(clang_getCursorSpelling(cursor)));
                    auto cursorKind = clang_getCursorKind(cursor);
                    switch (cursorKind)
                    {
                    case CXCursor_FunctionDecl:
                        data->functions_.push_back(FunctionDecl(cursor));
                        break;
                    case CXCursor_ClassDecl:
                    case CXCursor_StructDecl:
                        data->classes_.push_back(ClassDecl(cursor));
                        break;
                    default:
                        // do nothing
                        break;
                    }
                }
                return CXChildVisit_Continue;
            },
            this);
    }
    const std::vector<FunctionDecl> &functions() const
    {
        return functions_;
    }
    const std::vector<ClassDecl> &classes() const
    {
        return classes_;
    }
};

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

    TranslationUnit parsed_translation_unit(cursor, input_file_path);

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
    for (auto &function : parsed_translation_unit.functions())
    {
        output_file_stream << "    def(\"" << function.name() << "\", " << function.name() << ");\n";
    }
    for (auto &class_ : parsed_translation_unit.classes())
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