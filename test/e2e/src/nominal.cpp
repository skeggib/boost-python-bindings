#include "nominal.hpp"

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