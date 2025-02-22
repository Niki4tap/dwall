#include <string.h>
#include <stdlib.h>
#include "nonnull.h"

bool str_replace(char string nonnull_ptr, size_t string_size, const char from nonnull_ptr, const char to nonnull_ptr) {
	size_t from_len = strlen(from);
	size_t to_len = strlen(to);
	const char* end = string + strlen(string);
	long int diff_len = (long int)to_len - (long int)from_len;

	char* swap = malloc((size_t)(end - string));

	char* next = string;
	while ((next = strchr(next, from[0])) != NULL) {
		if (strncmp(from, next, from_len) != 0) {
			next++;
			continue;
		}

		if ((size_t)(end - string) + to_len > string_size) {
			free(swap);
			return false;
		}

		strncpy(swap, next + from_len, (size_t)(end - next));
		strncpy(next, to, to_len);
		strncpy(next + to_len, swap, (size_t)(end - next));
		next = next + to_len;
		end += diff_len;
	}

	free(swap);
	return true;
}
