#pragma once

#include <ostream>
#include <string>

#include "ast.hpp"

class FunctionExposer
{
private:
    std::string name_;

public:
    FunctionExposer(
        const std::string &name)
        : name_(name) {}
    const std::string &name() const { return name_; }
};

class ParameterExposer
{
private:
    std::string type_;

public:
    ParameterExposer(
        const std::string &type)
        : type_(type) {}
    const std::string &type() const { return type_; }
};

class ConstructorExposer
{
private:
    std::vector<ParameterExposer> parameters_;

public:
    ConstructorExposer(std::vector<ParameterExposer> parameters) : parameters_(parameters) {}
    const std::vector<ParameterExposer> &parameters() const { return parameters_; }
};

class MethodExposer
{
private:
    std::string name_;
    std::string fully_qualified_name_;

public:
    MethodExposer(
        std::string name,
        std::string fully_qualified_name)
        : name_(name),
          fully_qualified_name_(fully_qualified_name) {}
    const std::string &name() const { return name_; }
    const std::string &fully_qualified_name() const { return fully_qualified_name_; }
};

class FieldExposer
{
private:
    std::string name_;
    std::string fully_qualified_name_;

public:
    FieldExposer(
        std::string name,
        std::string fully_qualified_name)
        : name_(name),
          fully_qualified_name_(fully_qualified_name) {}
    const std::string &name() const { return name_; }
    const std::string &fully_qualified_name() const { return fully_qualified_name_; }
};

class ClassExposer
{
private:
    std::string name_;
    std::vector<ConstructorExposer> constructors_;
    std::vector<MethodExposer> methods_;
    std::vector<FieldExposer> fields_;

public:
    ClassExposer(
        const std::string &name,
        std::vector<ConstructorExposer> constructors,
        std::vector<MethodExposer> methods,
        std::vector<FieldExposer> fields)
        : name_(name),
          constructors_(constructors),
          methods_(methods),
          fields_(fields) {}
    const std::string &name() const { return name_; }
    const std::vector<ConstructorExposer> &constructors() const { return constructors_; };
    const std::vector<MethodExposer> &methods() const { return methods_; };
    const std::vector<FieldExposer> &fields() const { return fields_; };
};

class TranslationUnitExposer
{
private:
    std::string input_file_path_;
    std::vector<FunctionExposer> functions_;
    std::vector<ClassExposer> classes_;

public:
    TranslationUnitExposer(
        std::string input_file_path,
        std::vector<FunctionExposer> functions,
        std::vector<ClassExposer> classes)
        : input_file_path_(input_file_path),
          functions_(functions),
          classes_(classes) {}
    const std::string &input_file_path() const { return input_file_path_; }
    const std::vector<FunctionExposer> &functions() const { return functions_; }
    const std::vector<ClassExposer> &classes() const { return classes_; }
};

/**
 * @brief Convert a C++ translation unit into an exposer.
 */
TranslationUnitExposer expose(const TranslationUnit &translation_unit);

/**
 * @brief Generate boost::python bindings from a translation unit exposer.
 */
void generate(const TranslationUnitExposer &exposer, std::ostream &stream);
