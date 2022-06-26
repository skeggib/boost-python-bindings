#include "clang_parser.hpp"

#include "visitors.hpp"

#include <clang-c/Index.h>

AccessSpecifier convert_clang_access_specifier(CX_CXXAccessSpecifier access_specifier)
{
    switch (access_specifier)
    {
    case CX_CXXInvalidAccessSpecifier:
        return AccessSpecifier::InvalidAccessSpecifier;
    case CX_CXXPublic:
        return AccessSpecifier::Public;
    case CX_CXXProtected:
        return AccessSpecifier::Protected;
    case CX_CXXPrivate:
        return AccessSpecifier::Private;
    default:
        return AccessSpecifier::InvalidAccessSpecifier;
    }
}

FunctionDecl parse_function_decl(CXCursor cursor)
{
    lazy<std::string> name{
        [cursor]()
        { return clang_getCString(clang_getCursorSpelling(cursor)); }};
    return FunctionDecl(name);
}

ConstructorDecl parse_constructor_decl(CXCursor cursor)
{
    lazy<bool> is_copy{
        [cursor]()
        { return clang_CXXConstructor_isCopyConstructor(cursor); }};
    return ConstructorDecl(is_copy);
}

MethodDecl parse_method_decl(CXCursor cursor)
{
    lazy<std::string> name{
        [cursor]()
        { return clang_getCString(clang_getCursorSpelling(cursor)); }};
    return MethodDecl(name);
}

FieldDecl parse_field_decl(CXCursor cursor)
{
    lazy<std::string> name{
        [cursor]()
        { return clang_getCString(clang_getCursorSpelling(cursor)); }};
    lazy<AccessSpecifier> access_specifier{
        [cursor]()
        { return convert_clang_access_specifier(clang_getCXXAccessSpecifier(cursor)); }};
    return FieldDecl(name, access_specifier);
}

ClassDecl parse_class_decl(CXCursor cursor)
{
    return ClassDecl(
        lazy<std::string>(
            [cursor]()
            { return clang_getCString(clang_getCursorSpelling(cursor)); }),
        lazy<std::vector<MethodDecl>>(
            [cursor]()
            { return mapChildrenOfKind<MethodDecl>(cursor, CXCursor_CXXMethod, parse_method_decl); }),
        lazy<std::vector<FieldDecl>>(
            [cursor]()
            { return mapChildrenOfKind<FieldDecl>(cursor, CXCursor_FieldDecl, parse_field_decl); }),
        lazy<std::vector<ConstructorDecl>>(
            [cursor]()
            { return mapChildrenOfKind<ConstructorDecl>(cursor, CXCursor_Constructor, parse_constructor_decl); }));
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

TranslationUnit parse_translation_unit(CXCursor cursor, std::string input_file_path)
{
    VisitorData data(input_file_path);

    // clang_getSpellingLocation segfaults when used inside a lazy object, so we
    // have to parse the children of the translation unit here for now
    clang_visitChildren(
        cursor,
        visitor,
        &data);

    return TranslationUnit(
        lazy<std::vector<FunctionDecl>>(
            [data]()
            { return data.functions; }),
        lazy<std::vector<ClassDecl>>(
            [data]()
            { return data.classes; }));
}

TranslationUnit parse(const std::string &input_file_path)
{
    CXIndex index = clang_createIndex(0, 0);
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

    auto parsed = parse_translation_unit(cursor, input_file_path);

    clang_disposeTranslationUnit(translation_unit);
    clang_disposeIndex(index);

    return parsed;
}
