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

struct Date
{
    int day;
    int month;
    int year;

    Date();
    Date(int day, int month, int year);

    /**
     * @brief Get the date in the dd/mm/yyyy format;
     */
    std::string toString() const;
};

class Person
{
public:
    std::string firstName;
    std::string lastName;
    std::string toString() const;
};