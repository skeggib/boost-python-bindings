#pragma once

#include <string>

/**
 * @brief A class that has defaulted default and copy constructors.
 */
class DefaultedConstructors
{
public:
    DefaultedConstructors() = default;
    DefaultedConstructors(const DefaultedConstructors&) = default;

    std::string value;
};