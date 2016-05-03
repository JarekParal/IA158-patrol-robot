#pragma once

#include <cstdint>
#include "ev3api.h"
#include <ctype.h>

extern FILE *bt;

struct Target {
    int16_t position;
    int16_t distance;
};