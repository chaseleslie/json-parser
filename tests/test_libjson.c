#include "../src/json.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void exit_failure(int ret) {
	exit(ret);
}

int main() {
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
		exit_failure(retVal);
	}
	json_value* topVal = json_parser_parse(parserState, jsonStr, jsonStrLen);
	if (!topVal) {
		retVal = 1;
		exit_failure(retVal);
	}
	
	char buf[BUFSIZ];
	setbuf(stderr, buf);
	parserState->jsonStrPos = 26u;
	json_error_lineno("%zu:%zu", parserState);
	if (strncmp("0:26", buf, 4)) {
		retVal = 1;
		fclose(stderr);
		exit_failure(retVal);
	}
	//Buffer must be valid when stream closed (setbuf)
	fclose(stderr);
	return retVal;
}
