#pragma once
#include "nonnull.h"

typedef struct args {
	char* webhooks;
	char* message;
	bool is_short;
	bool display_help;
	bool verbose;
	bool display_version;

	char* unknown_argument;
} args_t;

args_t parse_args(char* argv[]);

void print_help(const char name nonnull_ptr);

void print_version();

