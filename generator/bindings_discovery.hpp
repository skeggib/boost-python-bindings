#pragma once

#include <functional>

void register_binder(std::function<void()> binder);
void bind_all();

#define REGISTER_BINDER(binder) namespace\
{\
    struct Register\
    {\
        Register()\
        {\
            register_binder(binder);\
        }\
    };\
    Register r;\
}
