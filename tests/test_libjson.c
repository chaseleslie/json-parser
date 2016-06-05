#include "../src/json.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void exit_failure(int ret) {
	exit(ret);
}

static int test_stdin() {
	int retVal = 1;
	
	size_t buffIncr = 4096;
	size_t buffLen = buffIncr;
	char* buff = malloc(sizeof(char) * buffLen);
	if (!buff) {
		retVal = 99;
		fprintf(stdout, "%s", "ERROR:\tmalloc()\n");
		exit_failure(retVal);
	}
	char* tmp = buff;
	size_t bytesRead = 0;
	size_t bytesReadTotal = 0;
	
	while ((bytesRead = fread(tmp, sizeof(char), buffIncr, stdin))) {
		bytesReadTotal += bytesRead;
		
		if (bytesRead != buffLen) {
			if (feof(stdin)) {
				break;
			} else {
				retVal = 99;
				fprintf(stdout, "%s", "ERROR:\tfread()\n");
				exit_failure(retVal);
			}
		} else {
			buffLen += buffIncr;
			buff = realloc(buff, buffLen);
			if (!buff) {
				retVal = 99;
				fprintf(stdout, "%s", "ERROR:\trealloc()\n");
				exit_failure(retVal);
			}
			tmp = buff + bytesReadTotal;
		}
	}
	
	if (buffLen > bytesReadTotal) {
		buff[bytesReadTotal] = 0;
	} else {
		buffLen += 8;
		buff = realloc(buff, buffLen);
		if (!buff) {
			retVal = 99;
			fprintf(stdout, "%s", "ERROR:\trealloc()\n");
			exit_failure(retVal);
		}
		buff[bytesReadTotal] = 0;
	}
	
	json_parser_state* parserState = json_parser_init(NULL, NULL);
	if (!parserState) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_init()\n");
		exit_failure(retVal);
	}
	
	json_value* topVal = json_parser_parse(parserState, buff, bytesReadTotal);
	if (!topVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse()\n");
		exit_failure(retVal);
	} else if (strncmp("complete", json_parser_get_state_string(parserState), 8)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_init(): parser state != complete\n");
		fprintf(stdout, "\tparserState->state:\t%s\n", json_parser_get_state_string(parserState));
		exit_failure(retVal);
	}
	
	free(buff);
	retVal = 0;
	return retVal;
}

int main(int argc, char** argv) {
	if (argc > 1 && !strncmp("--stdin", argv[1], 7)) {
		return test_stdin();
	}
	
	int retVal = 0;
	const char* jsonStr = (
		"{"
		"\"obj\": {\"arr\": [1,2,3]},"
		"\"str\": \"Some string here.\","
		"\"num\": 3.1415926535897932,"
		"\"tru\": true,"
		"\"fals\": false,"
		"\"nul\": null"
		"}"
	);
	const size_t jsonStrLen = strlen(jsonStr);
	
	json_parser_state* parserState = json_parser_init(NULL, NULL);
	if (!parserState) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_init()\n");
		exit_failure(retVal);
	}
	
	json_value* topVal = json_parser_parse(parserState, jsonStr, jsonStrLen);
	if (!topVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse()\n");
		exit_failure(retVal);
	} else if (strncmp("complete", json_parser_get_state_string(parserState), 8)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_init(): parser state != complete\n");
		fprintf(stdout, "\tparserState->state:\t%s\n", json_parser_get_state_string(parserState));
		exit_failure(retVal);
	}
	
	retVal = json_visitor_free_all(parserState, topVal);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_visitor_free_value()\n");
		exit_failure(retVal);
	}
	
	const char* escapedStr = "Some\\uD834\\uDD1EString";
	const size_t escapedStrLen = strlen(escapedStr);
	const char* unescapedStr = json_utils_unescape_string(parserState, escapedStr, escapedStrLen, &retVal);
	if (!unescapedStr || retVal) {
		retVal = 99;
		fprintf(stdout, "%s", "ERROR:\tjson_utils_unescape_string()\n");
		exit_failure(retVal);
	} else if (strncmp("Some\xF0\x9D\x84\x9EString", unescapedStr, 12)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_utils_unescape_string(): invalid unescape of UTF-16 surrogate pair\n");
		exit_failure(retVal);
	}
	
	char buf[BUFSIZ];
	setbuf(stderr, buf);
	parserState->jsonStrPos = 26u;
	json_error_lineno("%zu:%zu", parserState);
	if (strncmp("0:26", buf, 4)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_error_lineno()\n");
		fclose(stderr);
		exit_failure(retVal);
	}
	//Buffer must be valid when stream closed (setbuf)
	fclose(stderr);
	
	retVal = json_parser_clear(parserState);
	if (retVal) {
		fprintf(stdout, "%s", "FAIL:\tjson_parser_clear()\n");
		exit_failure(retVal);
	}
	
	return retVal;
}