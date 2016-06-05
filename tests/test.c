#include "../src/json.h"

#include <stdio.h>
#include <string.h>

int main() {
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
	char buf[BUFSIZ];
	setbuf(stderr, buf);
	
	json_parser_state* parserState = json_parser_init(NULL, NULL);
	json_value* topVal = json_parser_parse(parserState, jsonStr, jsonStrLen);
	
	//Buffer must be valid when stream closed
	fclose(stderr);
	return 0;
}
