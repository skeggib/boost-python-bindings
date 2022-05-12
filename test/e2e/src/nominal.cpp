#include "nominal.hpp"

#include <iomanip>
#include <sstream>

void doNothing() { }

std::string helloWorld()
{
    return "Hello world";
}

int add(int lhs, int rhs)
{
    return lhs + rhs;
}

void World::set(std::string msg) { this->msg_ = msg; }
std::string World::greet() { return msg_; }

Date::Date() 
    : day(1)
    , month(1)
    , year(1970)
{ }

Date::Date(int day, int month, int year)
    : day(day)
    , month(month)
    , year(year)
{ }

std::string Date::toString() const
{
    std::stringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << day << "/"
       << std::setw(2) << month << "/"
       << std::setw(4) << year;
    return ss.str();
}