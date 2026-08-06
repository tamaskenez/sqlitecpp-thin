#pragma once

#define CHECK(COND)                                  \
    do {                                             \
        if (!(COND)) {                               \
            throw std::logic_error("CHECK failed."); \
        }                                            \
    } while (false)
