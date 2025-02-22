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
		fprintf(stderr, "Unknown argument: %s\n", args.unknown_argument);
		print_help(*argv);
		return -1;
	}

	if (args.display_help) {
		print_help(*argv);
		return 0;
	}

	if (args.display_version) {
		print_version();
		return 0;
	}

	CURLcode res;
	res = curl_global_init(CURL_GLOBAL_ALL);
	if (res != CURLE_OK) {
		fprintf(stderr, "Couldn't initialize curl: %s\n", curl_easy_strerror(res));
		return -1;
	}

	char* s = args.message ?: read_stdin();
	char* announcement = make_announcement(s, args.is_short);
	if (announcement == NULL) {
		return -1;
	}
	if (args.message == NULL) { free(s); }

	size_t len = strlen(announcement);
	if (len > 2000) {
		fprintf(stderr, "warning: The message looks too long (expected max 2000 characters, got %zu), the string sanitization might fail, or most likely the discord endpoint will reject the message.\n", len);
	}

	bool ok = false;
	ok = str_replace(announcement, STR_SIZE, "\"", "\\\"");
	if (!ok) {
		fputs("Exceeded memory while sanitizing the message, check your message length\n", stderr);
		return -1;
	}
	ok = str_replace(announcement, STR_SIZE, "\n", "\\n");
	if (!ok) {
		fputs("Exceeded memory while sanitizing the message, check your message length\n", stderr);
		return -1;
	}

	char* json = malloc(STR_SIZE);

	int sz = snprintf(json, STR_SIZE, json_template, announcement);
	if (sz < 0) {
		fputs("Error formatting the message (message might be too long?)\n", stderr);
		return -1;
	}

	CURL* curl = curl_easy_init();
	if (!curl) {
		fputs("Couldn't make an instance of curl\n", stderr);
		return -1;
	}

	res = curl_easy_setopt(curl, CURLOPT_VERBOSE, args.verbose);
	if (res != CURLE_OK) {
		fprintf(stderr, "Couldn't set curl option: %s\n", curl_easy_strerror(res));
		return -1;
	}

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
	if (res != CURLE_OK) {
		fprintf(stderr, "Couldn't set curl option: %s\n", curl_easy_strerror(res));
		return -1;
	}

	struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if (res != CURLE_OK) {
		fprintf(stderr, "Couldn't set curl option: %s\n", curl_easy_strerror(res));
		return -1;
	}

	if (args.webhooks == NULL) {
		fprintf(stderr, "No webhooks provided, nothing to be done, have you forgotten to supply the `--webhooks` option?\n");
		return -1;
	}
	char* url = strtok(args.webhooks, ",");
	while (url != NULL) {
		res = curl_easy_setopt(curl, CURLOPT_URL, url);
		if (res != CURLE_OK) {
			fprintf(stderr, "Couldn't set curl option: %s\n", curl_easy_strerror(res));
			return -1;
		}

		curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "Error performing curl request: %s\n", curl_easy_strerror(res));
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

	int sz;
	if (!is_short) {
		sz = snprintf(s, STR_SIZE, message_template, getlogin(), hostname, ttyname(STDIN_FILENO), time_str, message);
	} else {
		sz = snprintf(s, STR_SIZE, short_message_template, message);
	}

	if (sz < 0) {
		fputs("Error formatting the message (message might be too long?)\n", stderr);
		return NULL;
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
	puts("");

	return original;
}
