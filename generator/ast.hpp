#pragma once

#include <memory>
#include <string>
#include <vector>

#include "lazy.hpp"

enum class AccessSpecifier
{
    Public,
    Protected,
    Private,
    InvalidAccessSpecifier
};

class FunctionDecl
{
private:
    lazy<std::string> name_;

public:
    FunctionDecl(lazy<std::string> name)
        : name_(name) {}
    const std::string &name() const { return name_; };
};

class ConstructorDecl
{
private:
    lazy<bool> is_copy_;

public:
    ConstructorDecl(lazy<bool> is_copy)
        : is_copy_(is_copy) {}
    bool is_copy() const { return is_copy_; };
};

class MethodDecl
{
private:
    lazy<std::string> name_;

public:
    MethodDecl(lazy<std::string> name)
        : name_(name) {}
    const std::string &name() const { return name_; };
};

class FieldDecl
{
private:
    lazy<std::string> name_;
    lazy<AccessSpecifier> access_specifier_;

public:
    FieldDecl(
        lazy<std::string> name,
        lazy<AccessSpecifier> access_specifier)
        : name_(name),
          access_specifier_(access_specifier) {}
    const std::string &name() const { return name_; };
    AccessSpecifier access_specifier() const { return access_specifier_; };
};

class ClassDecl
{
private:
    lazy<std::string> name_;
    lazy<std::vector<MethodDecl>> methods_;
    lazy<std::vector<FieldDecl>> fields_;
    lazy<std::vector<ConstructorDecl>> explicit_constructors_;

public:
    ClassDecl(
        lazy<std::string> name,
        lazy<std::vector<MethodDecl>> methods,
        lazy<std::vector<FieldDecl>> fields,
        lazy<std::vector<ConstructorDecl>> explicit_constructors)
        : name_(name),
          methods_(methods),
          fields_(fields),
          explicit_constructors_(explicit_constructors) {}
    const std::string &name() const { return name_; };
    const std::vector<FieldDecl> &fields() const { return fields_; };
    const std::vector<MethodDecl> &methods() const { return methods_; };
    const std::vector<ConstructorDecl> &explicit_constructors() const { return explicit_constructors_; };
    bool has_copy_constructor() const { return true; }; // TODO: test and implement
};

class TranslationUnit
{
private:
    std::string path_;
    lazy<std::vector<FunctionDecl>> functions_;
    lazy<std::vector<ClassDecl>> classes_;

public:
    TranslationUnit(
        std::string path,
        lazy<std::vector<FunctionDecl>> functions,
        lazy<std::vector<ClassDecl>> classes)
        : path_(path),
          functions_(functions),
          classes_(classes) {}
    const std::string &path() const { return path_; }
    const std::vector<FunctionDecl> &functions() const { return functions_; };
    const std::vector<ClassDecl> &classes() const { return classes_; };
};
