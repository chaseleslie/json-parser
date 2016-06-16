/* Copyright (C) 2015-2016 Chase
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../src/json.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static void exit_failure(int ret) {
	exit(ret);
}

static int test_json_pointer(json_parser_state* parserState) {
	int retVal = 1;
	
	const char* jsonStr = (
		"{"
			"\"obj\": {\"arr\": [1,2,3]},"
			"\"str\": \"Some string here.\","
			"\"num\": 3.1415926535897932,"
			"\"tru\": true,"
			"\"fals\": false,"
			"\"nul\": null,"
			"\"bad\\u0000wolf\": 1"
		"}"
	);
	const size_t jsonStrLen = strlen(jsonStr);
	
	retVal = json_parser_reset(parserState);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_reset()\n");
		exit_failure(retVal);
	}
	
	json_value* topVal = json_parser_parse(parserState, jsonStr, jsonStrLen);
	if (!topVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse()\n");
		exit_failure(retVal);
	} else if (strncmp("complete", json_parser_get_state_string(parserState), 8)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): parser state != complete\n");
		fprintf(stdout, "\tparserState->state:\t%s\n", json_parser_get_state_string(parserState));
		exit_failure(retVal);
	}
	
	const char* query = "/obj/arr/0";
	size_t queryLen = strlen(query);
	json_value* val = json_value_query(parserState, topVal, query, queryLen);
	if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (1)\n");
		exit_failure(retVal);
	} else if (val->valueType != number_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (1)\n");
		exit_failure(retVal);
	}
	json_number* num = val->value;
	if (num->value != 1.0) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (1)\n");
		exit_failure(retVal);
	}
	val = NULL;
	num = NULL;
	
	query = "/obj/arr/1";
	queryLen = strlen(query);
	val = json_value_query(parserState, topVal, query, queryLen);
	if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (2)\n");
		exit_failure(retVal);
	} else if (val->valueType != number_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (2)\n");
		exit_failure(retVal);
	}
	num = val->value;
	if (num->value != 2.0) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (2)\n");
		exit_failure(retVal);
	}
	val = NULL;
	num = NULL;
	
	query = "/obj/arr/2";
	queryLen = strlen(query);
	val = json_value_query(parserState, topVal, query, queryLen);
	if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (3)\n");
		exit_failure(retVal);
	} else if (val->valueType != number_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (3)\n");
		exit_failure(retVal);
	}
	num = val->value;
	if (num->value != 3.0) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (3)\n");
		exit_failure(retVal);
	}
	val = NULL;
	num = NULL;
	
	query = "/obj/str";
	queryLen = strlen(query);
	val = json_value_query(parserState, topVal, query, queryLen);
	if (val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (4)\n");
		exit_failure(retVal);
	}
	val = NULL;
	
	query = "/str";
	queryLen = strlen(query);
	val = json_value_query(parserState, topVal, query, queryLen);
	 if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (5)\n");
		exit_failure(retVal);
	} else if (val->valueType != string_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (5)\n");
		exit_failure(retVal);
	}
	json_string* str = val->value;
	if (strncmp(str->value, "Some string here.", 17)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (5)\n");
		exit_failure(retVal);
	}
	val = NULL;
	str = NULL;
	
	query = "/num";
	queryLen = strlen(query);
	val = json_value_query(parserState, topVal, query, queryLen);
	 if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (6)\n");
		exit_failure(retVal);
	} else if (val->valueType != number_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (6)\n");
		exit_failure(retVal);
	}
	num = val->value;
	if (num->value != 3.1415926535897932) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (6)\n");
		exit_failure(retVal);
	}
	val = NULL;
	num = NULL;
	
	query = "/tru";
	queryLen = strlen(query);
	val = json_value_query(parserState, topVal, query, queryLen);
	 if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (7)\n");
		exit_failure(retVal);
	} else if (val->valueType != true_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (7)\n");
		exit_failure(retVal);
	}
	val = NULL;
	
	query = "/fals";
	queryLen = strlen(query);
	val = json_value_query(parserState, topVal, query, queryLen);
	 if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (8)\n");
		exit_failure(retVal);
	} else if (val->valueType != false_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (8)\n");
		exit_failure(retVal);
	}
	val = NULL;
	
	query = "/nul";
	queryLen = strlen(query);
	val = json_value_query(parserState, topVal, query, queryLen);
	 if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (9)\n");
		exit_failure(retVal);
	} else if (val->valueType != null_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (9)\n");
		exit_failure(retVal);
	}
	val = NULL;
	
	const char query2[] = {'/', 'b', 'a', 'd', '\0', 'w', 'o', 'l', 'f'};
	queryLen = 9;
	val = json_value_query(parserState, topVal, query2, queryLen);
	 if (!val) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): error during query (10)\n");
		exit_failure(retVal);
	} else if (val->valueType != number_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (10)\n");
		exit_failure(retVal);
	}
	num = val->value;
	if (num->value != 1.0) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_value_query(): query returned invalid (10)\n");
		exit_failure(retVal);
	}
	val = NULL;
	num = NULL;
	
	retVal = json_visitor_free_all(parserState, topVal);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_visitor_free_all()\n");
		exit_failure(retVal);
	}
	
	retVal = 0;
	return retVal;
}

static int test_stdin(int shouldPass) {
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
		if (shouldPass) {
			fprintf(stdout, "%s", "FAIL:\tjson_parser_parse()\n");
			exit_failure(retVal);
		} else {
			fprintf(stdout, "%s", "XFAIL:\tjson_parser_parse()\n");
		}
	} else if (strncmp("complete", json_parser_get_state_string(parserState), 8)) {
		retVal = 1;
		const char* failStr = shouldPass ? "FAIL" : "XFAIL";
		fprintf(stdout, "%s%s", failStr, ":\tjson_parser_init(): parser state != complete\n");
		fprintf(stdout, "\tparserState->state:\t%s\n", json_parser_get_state_string(parserState));
		if (shouldPass) {
			exit_failure(retVal);
		}
	} else if (topVal && !shouldPass) {
		retVal = shouldPass;
		fprintf(stdout, "%s", "XPASS:\tjson_parser_init()\n");
		exit_failure(retVal);
	}
	
	free(buff);
	retVal = 0;
	return retVal;
}

int main(int argc, char** argv) {
	if (argc > 2 && !strncmp("--stdin", argv[1], 7)) {
		if (!strncmp("PASS", argv[2], 4)) {
			return test_stdin(1);
		} else {
			return test_stdin(0);
		}
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
	
	/* Test json_parser_init() with default alloc */
	json_parser_state* parserState = json_parser_init(NULL, NULL);
	if (!parserState) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_init()\n");
		exit_failure(retVal);
	}
	
	/* Test json_parser_parse() with basic JSON str with default alloc */
	json_value* topVal = json_parser_parse(parserState, jsonStr, jsonStrLen);
	if (!topVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse()\n");
		exit_failure(retVal);
	} else if (strncmp("complete", json_parser_get_state_string(parserState), 8)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): parser state != complete\n");
		fprintf(stdout, "\tparserState->state:\t%s\n", json_parser_get_state_string(parserState));
		exit_failure(retVal);
	}
	
	json_object* obj = topVal->value;
	if (topVal->valueType != object_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): top value not an object\n");
		exit_failure(retVal);
	} else if (!obj || obj->size != 6) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): top object parsed incorrectly\n");
		exit_failure(retVal);
	} else if (obj->names[0]->valueLen != 3 || strncmp(obj->names[0]->value, "obj", 3)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): top object names incorrect\n");
		exit_failure(retVal);
	} else if (obj->names[1]->valueLen != 3 || strncmp(obj->names[1]->value, "str", 3)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): top object names incorrect\n");
		exit_failure(retVal);
	} else if (obj->names[2]->valueLen != 3 || strncmp(obj->names[2]->value, "num", 3)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): top object names incorrect\n");
		exit_failure(retVal);
	} else if (obj->names[3]->valueLen != 3 || strncmp(obj->names[3]->value, "tru", 3)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): top object names incorrect\n");
		exit_failure(retVal);
	} else if (obj->names[4]->valueLen != 4 || strncmp(obj->names[4]->value, "fals", 3)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): top object names incorrect\n");
		exit_failure(retVal);
	} else if (obj->names[5]->valueLen != 3 || strncmp(obj->names[5]->value, "nul", 3)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): top object names incorrect\n");
		exit_failure(retVal);
	}
	json_object* obj2 = obj->values[0]->value;
	if (obj->values[0]->valueType != object_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): nested object parsed incorrectly\n");
		exit_failure(retVal);
	} else if (!obj2 || obj2->size != 1) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): nested object parsed incorrectly\n");
		exit_failure(retVal);
	}
	json_array* arr = obj2->values[0]->value;
	if (obj2->names[0]->valueLen != 3 || strncmp(obj2->names[0]->value, "arr", 3) || obj2->values[0]->valueType != array_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): nested object parsed incorrectly\n");
		exit_failure(retVal);
	} else if (!arr || arr->size != 3) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): array parsed incorrectly\n");
		exit_failure(retVal);
	}
	if (obj->values[1]->valueType != string_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): string parsed incorrectly\n");
		exit_failure(retVal);
	} else if (obj->values[2]->valueType != number_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): number parsed incorrectly\n");
		exit_failure(retVal);
	} else if (obj->values[3]->valueType != true_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): true parsed incorrectly\n");
		exit_failure(retVal);
	} else if (obj->values[4]->valueType != false_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): false parsed incorrectly\n");
		exit_failure(retVal);
	} else if (obj->values[5]->valueType != null_value) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): null parsed incorrectly\n");
		exit_failure(retVal);
	}
	
	/* Test json_visitor_free_all() with default alloc */
	retVal = json_visitor_free_all(parserState, topVal);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_visitor_free_all()\n");
		exit_failure(retVal);
	}
	topVal = NULL;
	
	/* Test json_parser_reset() and parsing/freeing again */
	retVal = json_parser_reset(parserState);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_reset()\n");
		exit_failure(retVal);
	}
	topVal = json_parser_parse(parserState, jsonStr, jsonStrLen);
	if (!topVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse() after json_parser_reset()\n");
		exit_failure(retVal);
	} else if (strncmp("complete", json_parser_get_state_string(parserState), 8)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse() after json_parser_reset(): parser state != complete\n");
		fprintf(stdout, "\tparserState->state:\t%s\n", json_parser_get_state_string(parserState));
		exit_failure(retVal);
	}
	retVal = json_visitor_free_all(parserState, topVal);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_visitor_free_all() after json_parser_reset()\n");
		exit_failure(retVal);
	}
	topVal = NULL;
	
	/* Test handling escaped control chars in strings */
	const char* jsonStr2 = "{\"bad\\u0000wolf\": 1}";
	const size_t jsonStr2Len = strlen(jsonStr2);
	retVal = json_parser_reset(parserState);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_reset()\n");
		exit_failure(retVal);
	}
	topVal = json_parser_parse(parserState, jsonStr2, jsonStr2Len);
	if (!topVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse() after json_parser_reset()\n");
		exit_failure(retVal);
	} else if (strncmp("complete", json_parser_get_state_string(parserState), 8)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse() after json_parser_reset(): parser state != complete\n");
		fprintf(stdout, "\tparserState->state:\t%s\n", json_parser_get_state_string(parserState));
		exit_failure(retVal);
	}
	json_object* obj3 = topVal->value;
	if (topVal->valueType != object_value || !obj3) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): handle control chars in strings (1)\n");
		exit_failure(retVal);
	} else if (obj3->names[0]->valueLen != 8 || memcmp(obj3->names[0]->value, "bad\000wolf", 8)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse(): handle control chars in strings (2)\n");
		fprintf(stdout, "\tstr len:\t%zu\n", obj3->names[0]->valueLen);
		fprintf(stdout, "\tstr:\t%s\n", obj3->names[0]->value);
		exit_failure(retVal);
	}
	retVal = json_visitor_free_all(parserState, topVal);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_visitor_free_all() after json_parser_reset()\n");
		exit_failure(retVal);
	}
	topVal = NULL;
	
	
	/* Test setting option json_max_nested_level */
	retVal = json_parser_reset(parserState);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_reset()\n");
		exit_failure(retVal);
	}
	retVal = json_parser_setopt(parserState, json_max_nested_level, 1);
	if (retVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_setopt()\n");
		exit_failure(retVal);
	}
	topVal = json_parser_parse(parserState, jsonStr, jsonStrLen);
	if (topVal) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_parser_parse() with opt json_max_nested_level == 1\n");
		retVal = json_visitor_free_all(parserState, topVal);
		if (retVal) {
			fprintf(stdout, "%s", "FAIL:\tjson_visitor_free_all()\n");
		}
		exit_failure(retVal);
	}
	topVal = NULL;
	
	/* Test unescape string to UTF-8 */
	const char* escapedStr = "Some\\uD834\\uDD1EString";
	const size_t escapedStrLen = strlen(escapedStr);
	size_t unescapedStrLen = 0;
	const char* unescapedStr = json_utils_unescape_string(parserState, escapedStr, escapedStrLen, &retVal, &unescapedStrLen);
	if (!unescapedStr || retVal) {
		retVal = 99;
		fprintf(stdout, "%s", "ERROR:\tjson_utils_unescape_string()\n");
		exit_failure(retVal);
	} else if (strncmp("Some""\xF0\x9D\x84\x9E""String", unescapedStr, 12)) {
		retVal = 1;
		fprintf(stdout, "%s", "FAIL:\tjson_utils_unescape_string(): invalid unescape of UTF-16 surrogate pair\n");
		exit_failure(retVal);
	}
	
	/* Test JSON Pointer query */
	retVal = test_json_pointer(parserState);
	if (retVal) {
		return retVal;
	}
	
	/* Test error reporting */
	char buf[BUFSIZ];
	setbuf(parserState->errorStream, buf);
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
