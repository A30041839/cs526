#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>
#include "types.hpp"

void print_debug(const char* info) {
    if (DEBUG) {
        printf("%s\n", info);
    }
}

#endif