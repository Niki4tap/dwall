#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "args.h"

const char no_banner_short_flag[] = "-n";
const char no_banner_flag[] = "--no-banner";

const char webhooks_short_flag[] = "-w";
const char webhooks_flag[] = "--webhooks";

const char help_short_flag[] = "-h";
const char help_flag[] = "--help";

const char verbose_short_flag[] = "-v";
const char verbose_flag[] = "--verbose";

const char version_short_flag[] = "-V";
const char version_flag[] = "--version";

args_t parse_args(char* argv[]) {
	char** it = argv + 1;
	args_t args = { 0 };

	while (*it != NULL) {
		#define flag_eq(x) strncmp(*it, x, (sizeof x - 1)) == 0
		#define next {it++; continue;}
		if (flag_eq(no_banner_short_flag) || flag_eq(no_banner_flag)) {
			args.is_short = true;
			next;
		}

		if (flag_eq(webhooks_short_flag) || flag_eq(webhooks_flag)) {
			args.webhooks = *++it;
			next;
		}

		if (flag_eq(help_short_flag) || flag_eq(help_flag)) {
			args.display_help = true;
			next;
		}

		if (flag_eq(verbose_short_flag) || flag_eq(verbose_flag)) {
			args.verbose = true;
			next;
		}

		if (flag_eq(version_short_flag) || flag_eq(version_flag)) {
			args.display_version = true;
			next;
		}

		if (it[1] == NULL) {
			args.message = *it;
			next;
		}

		args.unknown_argument = *it;
		return args;

		#undef flag_eq
		#undef next
	}

	return args;
}

void print_help(const char name nonnull_ptr) {
	printf("\
Usage: %s [options] -w <webhooks> [<message>]\n\
\n\
Write a message to discord.\n\
\n\
Options:\n\
	-n, --no-banner		do not print banner\n\
	-w, --webhooks		list of comma separated webhooks to broadcast to\n\
\n\
	-h, --help			display this help\n\
	-v, --verbose		display debug output\n\
	-V, --version		display version\n\
", name);
}

void print_version() {
	puts("dwall version " VERSION);
}
