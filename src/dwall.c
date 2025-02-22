#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include "nonnull.h"
#include "str.h"
#include "args.h"

const char* const message_template = "\
\\n\
Broadcast message from %s@%s (%s) (%s):\\n\
\\n\
%s\\n\
\\n";

const char* const short_message_template = "\n%s\n";

const char* const json_template = "\
{\
	\"content\": \"%s\",\
	\"allowed_mentions\": {\
		\"parse\": [\"roles\", \"users\", \"everyone\"]\
	}\
}";

#define STR_SIZE 8 * 1024

char* make_announcement(const char message nonnull_ptr, bool is_short);
char* read_stdin();

int main(int argc, char* argv[]) {
	(void)argc;
	args_t args = parse_args(argv);

	if (args.unknown_argument != NULL) {
		fprintf(stderr, "Unknown argument: %s", args.unknown_argument);
		return -1;
	}

	if (args.display_help) {
		print_help(*argv);
		return -1;
	}

	if (args.display_version) {
		print_version();
		return -1;
	}

	CURLcode res;
	res = curl_global_init(CURL_GLOBAL_ALL);
	if (res != CURLE_OK) {
		fprintf(stderr, "Couldn't initialize curl: %s", curl_easy_strerror(res));
		return -1;
	}

	char* s = args.message ?: read_stdin();
	char* announcement = make_announcement(s, args.is_short);
	if (args.message == NULL) { free(s); }
	str_replace(announcement, STR_SIZE, "\"", "\\\"");
	str_replace(announcement, STR_SIZE, "\n", "\\n");
	char* json = malloc(STR_SIZE);
	snprintf(json, STR_SIZE, json_template, announcement);

	CURL* curl = curl_easy_init();
	if (!curl) {
		fputs("Couldn't make an instance of curl", stderr);
		return -1;
	}

	res = curl_easy_setopt(curl, CURLOPT_VERBOSE, args.verbose);
	if (res != CURLE_OK) {
		fprintf(stderr, "Couldn't set curl option: %s", curl_easy_strerror(res));
		return -1;
	}

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
	if (res != CURLE_OK) {
		fprintf(stderr, "Couldn't set curl option: %s", curl_easy_strerror(res));
		return -1;
	}

	struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if (res != CURLE_OK) {
		fprintf(stderr, "Couldn't set curl option: %s", curl_easy_strerror(res));
		return -1;
	}

	char* url = strtok(args.webhooks, ",");
	while (url != NULL) {
		res = curl_easy_setopt(curl, CURLOPT_URL, url);
		if (res != CURLE_OK) {
			fprintf(stderr, "Couldn't set curl option: %s", curl_easy_strerror(res));
			return -1;
		}

		curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "Error performing curl request: %s", curl_easy_strerror(res));
			return -1;
		}

		url = strtok(NULL, ",");
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	free(json);
	free(announcement);

	return 0;
}

char* make_announcement(const char message nonnull_ptr, bool is_short) {
	char* s = malloc(STR_SIZE);

	char* hostname = malloc(STR_SIZE);
	gethostname(hostname, STR_SIZE);

	char* time_str = malloc(STR_SIZE);
	time_t t = time(NULL);
	strftime(time_str, STR_SIZE, "%c", localtime(&t));

	if (!is_short) {
		snprintf(s, STR_SIZE, message_template, getlogin(), hostname, ttyname(STDIN_FILENO), time_str, message);
	} else {
		snprintf(s, STR_SIZE, short_message_template, message);
	}

	free(time_str);
	free(hostname);

	return s;
}

char* read_stdin() {
	char* original = malloc(STR_SIZE);
	char* s = original;
	while (scanf("%79s", s) != EOF) {
		size_t scanned = strlen(s);
		s += scanned;
		*s = '\n';
		s++;
		*s = '\0';
	}

	return original;
}
