#pragma once

#include <string>

/**
 * @brief Literally do nothing
 */
void doNothing();

/**
 * @brief Get a string containing 'Hello world'
 */
std::string helloWorld();

/**
 * @brief Compute lhs + rhs and return the result
 */
int add(int lhs, int rhs);

/**
 * @brief Example class from boost::python tutorial
 */
class World
{
public:
    void set(std::string msg);
    std::string greet();
private:
    std::string msg_;
};