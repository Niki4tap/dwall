#pragma once
#include <stdlib.h>
#include "nonnull.h"

bool str_replace(char string nonnull_ptr, size_t string_size, const char to nonnull_ptr, const char from nonnull_ptr);

struct { char* element; char* next; } split_one(char string nonnull_ptr, char delim);
