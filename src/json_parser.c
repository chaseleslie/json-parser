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

#ifndef JSON_PARSER_C
#define JSON_PARSER_C


#define JSON_TOP_LVL 1

#include "json_parser.h"
#include "json_types.h"
#include "json_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


const size_t JSON_MAX_NESTED_DEFAULT = 128;

static void json_parser_skip_ws(json_parser_state* parserState);
static bool json_parser_expect(json_parser_state* parserState, const char c, const char* err);

static inline int json_parser_check_state(json_parser_state* parserState, int state);
static inline int json_parser_add_state(json_parser_state* parserState, int state);
static inline int json_parser_remove_state(json_parser_state* parserState, int state);


int json_parser_setopt(json_parser_state* parserState, JSON_PARSER_OPT opt, ...) {
	int retVal = 1;
	if (opt < 0 || opt >= JSON_PARSER_OPT_MAX || !parserState) {
		return retVal;
	}
	
	va_list args;
	va_start(args, opt);
	
	switch (opt) {
		case json_max_nested_level: {
			int maxLvl = va_arg(args, int);
			if (maxLvl > 0) {
				parserState->maxNestedLevel = maxLvl;
			} else {
				parserState->maxNestedLevel = JSON_MAX_NESTED_DEFAULT;
			}
		}
		break;
		case json_error_stream: {
			parserState->errorStream = va_arg(args, FILE*);
		}
		break;
		default:
		case JSON_PARSER_OPT_MAX:
			va_end(args);
			return retVal;
		break;
	}
	
	va_end(args);
	
	retVal = 0;
	return retVal;
}

json_parser_state* json_parser_init(alloc_function allocFunction, free_function freeFunction) {
	json_allocator* JSON_Allocator;
	json_factory* JSON_Factory;
	json_types_init(allocFunction, freeFunction, &JSON_Allocator, &JSON_Factory);
	
	json_parser_state* parserState = (json_parser_state*) JSON_Allocator->malloc(sizeof(json_parser_state));
	if (!parserState) {
		return NULL;
	}
	
	parserState->JSON_Allocator = JSON_Allocator;
	parserState->JSON_Factory = JSON_Factory;
	parserState->jsonStrPos = 0;
	parserState->state = init_state;
	parserState->nestedLevel = 0;
	parserState->maxNestedLevel = JSON_MAX_NESTED_DEFAULT;
	parserState->errorStream = stderr;
	
	return parserState;
}

int json_parser_clear(json_parser_state* parserState) {
	if (!parserState) {
		return 1;
	}
	
	free_function freeFunction = parserState->JSON_Allocator->free;
	freeFunction(parserState->JSON_Factory);
	freeFunction(parserState->JSON_Allocator);
	freeFunction(parserState);
	
	return 0;
}

int json_parser_reset(json_parser_state* parserState) {
	if (!parserState) {
		return 1;
	}
	
	parserState->jsonStr = NULL;
	parserState->jsonStrLength = 0;
	parserState->jsonStrPos = 0;
	parserState->state = init_state;
	parserState->nestedLevel = 0;
	parserState->maxNestedLevel = JSON_MAX_NESTED_DEFAULT;
	
	return 0;
}

json_value* json_parser_parse(json_parser_state* parserState, const char* jsonStr, size_t jsonStrLength) {
	if (!parserState || !jsonStr || !jsonStrLength) {
		return NULL;
	}
	
	parserState->jsonStr = jsonStr;
	parserState->jsonStrLength = jsonStrLength;
	
	json_value* topVal = json_parser_parse_value(parserState, NULL, unspecified_value);
	if (!topVal) {
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	json_parser_add_state(parserState, complete_state);
	
	return topVal;
}

//TODO: Check return values/edge cases/etc
json_value* json_parser_parse_value(json_parser_state* parserState, void* parentValue, JSON_VALUE parentValueType) {
	json_value* val = NULL;
	
	json_parser_skip_ws(parserState);
	if (!json_parser_expect(parserState, 0, "json_parser:%u:%u Error: Expecting value\n")) {
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	switch (parserState->jsonStr[parserState->jsonStrPos]) {
		case '{': {
			parserState->jsonStrPos += 1;
			if (parserState->nestedLevel < parserState->maxNestedLevel) {
				parserState->nestedLevel += 1;
			} else {
				json_parser_add_state(parserState, error_state);
				return NULL;
			}
			if (!json_parser_expect(parserState, 0, "json_parser:%u:%u Error: Expecting '}' or value\n")) {
				json_parser_add_state(parserState, error_state);
				return NULL;
			}
			val = parserState->JSON_Factory->new_json_value(parserState->JSON_Factory, object_value, NULL, unspecified_value, NULL);
			if (!val) {
				json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_value\n", parserState);
				json_parser_add_state(parserState, error_state);
				return NULL;
			}
			json_object* obj = json_parser_parse_object(parserState, val);
			if (!obj) {
				parserState->JSON_Allocator->free(val);
				return NULL;
			} else if (json_parser_check_state(parserState, error_state)) {
				json_visitor_free_object(parserState->JSON_Factory, obj);
				parserState->JSON_Allocator->free(val);
				return NULL;
			}
			val->value = obj;
			parserState->nestedLevel -= 1;
		}
		break;
		case '[': {
			parserState->jsonStrPos += 1;
			if (parserState->nestedLevel < parserState->maxNestedLevel) {
				parserState->nestedLevel += 1;
			} else {
				json_parser_add_state(parserState, error_state);
				return NULL;
			}
			if (!json_parser_expect(parserState, 0, "json_parser:%u:%u Error: Expecting ']' or value\n")) {
				json_parser_add_state(parserState, error_state);
				return NULL;
			}
			val = parserState->JSON_Factory->new_json_value(parserState->JSON_Factory, array_value, NULL, unspecified_value, NULL);
			if (!val) {
				json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_value\n", parserState);
				json_parser_add_state(parserState, error_state);
				return NULL;
			}
			json_array* arr = json_parser_parse_array(parserState, val);
			if (!arr) {
				parserState->JSON_Allocator->free(val);
				return NULL;
			} else if (json_parser_check_state(parserState, error_state)) {
				json_visitor_free_array(parserState->JSON_Factory, arr);
				parserState->JSON_Allocator->free(val);
				return NULL;
			}
			val->value = arr;
			parserState->nestedLevel -= 1;
		}
		break;
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '-': {
			val = parserState->JSON_Factory->new_json_value(parserState->JSON_Factory, number_value, NULL, unspecified_value, NULL);
			if (!val) {
				json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_value\n", parserState);
				json_parser_add_state(parserState, error_state);
				return NULL;
			}
			json_number* num = json_parser_parse_number(parserState, val);
			if (!num) {
				parserState->JSON_Allocator->free(val);
				return NULL;
			}
			val->value = num;
		}
		break;
		case '"': {
			parserState->jsonStrPos += 1;
			val = parserState->JSON_Factory->new_json_value(parserState->JSON_Factory, string_value, NULL, unspecified_value, NULL);
			if (!val) {
				json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_value\n", parserState);
				json_parser_add_state(parserState, error_state);
				return NULL;
			}
			json_string* str = json_parser_parse_string(parserState, val);
			if (!str) {
				parserState->JSON_Allocator->free(val);
				return NULL;
			}
			val->value = str;
		}
		break;
		case 't': {
			const size_t jsonStrPos = parserState->jsonStrPos;
			const char* jsonStr = parserState->jsonStr + jsonStrPos;
			if (parserState->jsonStrLength >= (jsonStrPos + 4) && strstr(jsonStr, JSON_VALUE_NAMES[true_value]) == jsonStr) {
				val = parserState->JSON_Factory->new_json_value(parserState->JSON_Factory, true_value, NULL, unspecified_value, NULL);
				if (!val) {
					json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_value\n", parserState);
					json_parser_add_state(parserState, error_state);
					return NULL;
				}
				json_true* tru = json_parser_parse_true(parserState, val);
				if (!tru) {
					parserState->JSON_Allocator->free(val);
					return NULL;
				}
				val->value = tru;
			} else {
				json_error_lineno("json_parser:%u:%u Expecting value\n", parserState);
				return NULL;
			}
		}
		break;
		case 'f': {
			const size_t jsonStrPos = parserState->jsonStrPos;
			const char* jsonStr = parserState->jsonStr + jsonStrPos;
			if (parserState->jsonStrLength >= (jsonStrPos + 5) && strstr(jsonStr, JSON_VALUE_NAMES[false_value]) == jsonStr) {
				val = parserState->JSON_Factory->new_json_value(parserState->JSON_Factory, false_value, NULL, unspecified_value, NULL);
				if (!val) {
					json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_value\n", parserState);
					json_parser_add_state(parserState, error_state);
					return NULL;
				}
				json_false* fals = json_parser_parse_false(parserState, val);
				if (!fals) {
					parserState->JSON_Allocator->free(val);
					return NULL;
				}
				val->value = fals;
			} else {
				json_error_lineno("json_parser:%u:%u Expecting value\n", parserState);
				return NULL;
			}
		}
		break;
		case 'n': {
			const size_t jsonStrPos = parserState->jsonStrPos;
			const char* jsonStr = parserState->jsonStr + jsonStrPos;
			if (parserState->jsonStrLength >= (jsonStrPos + 4) && strstr(jsonStr, JSON_VALUE_NAMES[null_value]) == jsonStr) {
				val = parserState->JSON_Factory->new_json_value(parserState->JSON_Factory, null_value, NULL, unspecified_value, NULL);
				if (!val) {
					json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_value\n", parserState);
					json_parser_add_state(parserState, error_state);
					return NULL;
				}
				json_null* nul = json_parser_parse_null(parserState, val);
				if (!nul) {
					parserState->JSON_Allocator->free(val);
					return NULL;
				}
				val->value = nul;
			} else {
				json_error_lineno("json_parser:%u:%u Expecting value\n", parserState);
				return NULL;
			}
		}
		break;
		default: {
			json_error_lineno("json_parser:%u:%u Expecting value\n", parserState);
			return NULL;
		}
		break;
	}
	
	if (parentValue) {
		val->parentValueType = parentValueType;
		val->parentValue = parentValue;
	} else {
		val->parentValueType = unspecified_value;
		val->parentValue = NULL;
	}
	
	return val;
}

json_object* json_parser_parse_object(json_parser_state* parserState, json_value* parentValue) {
	json_object* obj = NULL;
	json_parser_skip_ws(parserState);
	
	//Empty object case "{}"
	if (parserState->jsonStr[parserState->jsonStrPos] == JSON_TOKEN_NAMES[json_token_rbrace]) {
		obj = parserState->JSON_Factory->new_json_object(parserState->JSON_Factory, parentValue);
		if (!obj) {
			json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_object\n", parserState);
			json_parser_add_state(parserState, error_state);
			return NULL;
		}
		parserState->jsonStrPos += 1;
		return obj;
	}
	
	obj = parserState->JSON_Factory->new_json_object(parserState->JSON_Factory, parentValue);
	if (!obj) {
		json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_object\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	do {
		//Parse Pair: json_string ':' json_value
		json_parser_skip_ws(parserState);
		if (!json_parser_expect(parserState, '"', "json_parser:%u:%u Error: Expecting '\"'\n")) {
			json_parser_add_state(parserState, error_state);
			return obj;
		}
		parserState->jsonStrPos += 1;
		
		json_string* str = json_parser_parse_string(parserState, NULL);
		if (!str) {
			return obj;
		}
		
		json_parser_skip_ws(parserState);
		if (!json_parser_expect(parserState, ':', "json_parser:%u:%u Expecting ':'\n")) {
			json_parser_add_state(parserState, error_state);
			parserState->JSON_Allocator->free(str);
			return obj;
		}
		parserState->jsonStrPos += 1;
		json_parser_skip_ws(parserState);
		
		json_value* value = json_parser_parse_value(parserState, obj, object_value);
		if (!value) {
			json_parser_add_state(parserState, error_state);
			parserState->JSON_Allocator->free(str);
			return obj;
		}
		
		size_t ret = json_object_add_pair(parserState->JSON_Factory, obj, str, value);
		if (!ret) {
			json_error_lineno("json_parser:%u:%u Error: json_object_add_pair()\n", parserState);
			json_parser_add_state(parserState, error_state);
			return obj;
		}
		
		json_parser_skip_ws(parserState);
	} while (
		parserState->jsonStrPos < parserState->jsonStrLength
		&& parserState->jsonStr[parserState->jsonStrPos] == JSON_TOKEN_NAMES[json_token_comma]
		&& parserState->jsonStrPos++
	);
	
	json_parser_skip_ws(parserState);
	if (!json_parser_expect(parserState, '}', "json_parser:%u:%u Expecting '}'\n")) {
		json_parser_add_state(parserState, error_state);
		return obj;
	}
	parserState->jsonStrPos += 1;
	
	return obj;
}

json_array* json_parser_parse_array(json_parser_state* parserState, json_value* parentValue) {
	json_array* arr = NULL;
	json_parser_skip_ws(parserState);
	
	//Empty array case "[]"
	if (parserState->jsonStr[parserState->jsonStrPos] == JSON_TOKEN_NAMES[json_token_rbrack]) {
		arr = parserState->JSON_Factory->new_json_array(parserState->JSON_Factory, parentValue);
		if (!arr) {
			json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_array\n", parserState);
			json_parser_add_state(parserState, error_state);
			return NULL;
		}
		parserState->jsonStrPos += 1;
		return arr;
	}
	
	arr = parserState->JSON_Factory->new_json_array(parserState->JSON_Factory, parentValue);
	if (!arr) {
		json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_array\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	do {
		//Parse List: json_value [',' json_value]
		json_parser_skip_ws(parserState);
		
		json_value* val = json_parser_parse_value(parserState, arr, array_value);
		if (!val) {
			json_error_lineno("json_parser:%u:%u Expecting value\n", parserState);
			json_parser_add_state(parserState, error_state);
			return arr;
		}
		
		size_t ret = json_array_add_element(parserState->JSON_Factory, arr, val);
		if (!ret) {
			json_error_lineno("json_parser:%u:%u Error: json_array_add_element()\n", parserState);
			json_parser_add_state(parserState, error_state);
			return arr;
		}
		
		json_parser_skip_ws(parserState);
	} while (
		parserState->jsonStrPos < parserState->jsonStrLength
		&& parserState->jsonStr[parserState->jsonStrPos] == JSON_TOKEN_NAMES[json_token_comma]
		&& parserState->jsonStrPos++
	);
	
	json_parser_skip_ws(parserState);
	if (!json_parser_expect(parserState, ']', "json_parser:%u:%u Expecting ']'\n")) {
		json_parser_add_state(parserState, error_state);
		return arr;
	}
	parserState->jsonStrPos += 1;
	
	return arr;
}

json_number* json_parser_parse_number(json_parser_state* parserState, json_value* parentValue) {
	json_number* num = NULL;
	double d = 0.0;
	char* endNum = NULL;
	d = strtod(parserState->jsonStr + parserState->jsonStrPos, &endNum);
	if (!d && (endNum == parserState->jsonStr + parserState->jsonStrPos)) {
		json_error_lineno("json_parser:%u:%u Expecting number\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	num = parserState->JSON_Factory->new_json_number(parserState->JSON_Factory, d, parentValue);
	if (!num) {
		json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_number\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	parserState->jsonStrPos += endNum - (parserState->jsonStr + parserState->jsonStrPos);
	return num;
}

json_string* json_parser_parse_string(json_parser_state* parserState, json_value* parentValue) {
	json_string* str = NULL;
	bool foundEndQuote = false;
	size_t startPos = parserState->jsonStrPos;
	
	while (parserState->jsonStrPos < parserState->jsonStrLength) {
		if (parserState->jsonStr[parserState->jsonStrPos] == JSON_TOKEN_NAMES[json_token_quote]) {
			if (parserState->jsonStrPos > startPos && parserState->jsonStr[parserState->jsonStrPos - 1] == JSON_TOKEN_NAMES[json_token_backslash]) {
				//Escaped quote
			} else {
				foundEndQuote = true;
				break;
			}
		} else if ((uint8_t) parserState->jsonStr[parserState->jsonStrPos] <= 0x1F) {
			json_error_lineno("json_parser:%u:%u Invalid control character in string\n", parserState);
			json_parser_add_state(parserState, error_state);
			return NULL;
		}
		parserState->jsonStrPos += 1;
	}
	
	if (!foundEndQuote) {
		json_error_lineno("json_parser:%u:%u Expecting '\"', reached eos\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	size_t dataLen = parserState->jsonStrPos - startPos;
	int ret = 0;
	size_t unescapedLen = 0;
	char* data = json_utils_unescape_string(parserState, parserState->jsonStr + startPos, dataLen, &ret, &unescapedLen);
	if (!data || ret) {
		json_error_lineno("json_parser:%u:%u Error: json_utils_unescape_string()", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	str = parserState->JSON_Factory->new_json_string(parserState->JSON_Factory, data, unescapedLen, parentValue);
	if (!str) {
		json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_string\n", parserState);
		json_parser_add_state(parserState, error_state);
		parserState->JSON_Allocator->free(data);
		return NULL;
	}
	
	parserState->jsonStrPos += 1;
	return str;
}

json_true* json_parser_parse_true(json_parser_state* parserState, json_value* parentValue) {
	json_true* tru = parserState->JSON_Factory->new_json_true(parserState->JSON_Factory, parentValue);
	if (!tru) {
		json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_true\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	parserState->jsonStrPos += 4;
	return tru;
}

json_false* json_parser_parse_false(json_parser_state* parserState, json_value* parentValue) {
	json_false* fals = parserState->JSON_Factory->new_json_false(parserState->JSON_Factory, parentValue);
	if (!fals) {
		json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_false\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	parserState->jsonStrPos += 5;
	return fals;
}

json_null* json_parser_parse_null(json_parser_state* parserState, json_value* parentValue) {
	json_null* nul = parserState->JSON_Factory->new_json_null(parserState->JSON_Factory, parentValue);
	if (!nul) {
		json_error_lineno("json_parser:%u:%u Error: JSON_Factory::new_json_null\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	parserState->jsonStrPos += 4;
	return nul;
}

static void json_parser_skip_ws(json_parser_state* parserState) {
	while (parserState->jsonStrPos < parserState->jsonStrLength && isspace(parserState->jsonStr[parserState->jsonStrPos])) {
		parserState->jsonStrPos += 1;
	}
}

//Pass nul byte for c to not check the char, just compare pos to len
static bool json_parser_expect(json_parser_state* parserState, const char c, const char* err) {
	if (!(parserState->jsonStrPos < parserState->jsonStrLength) || (c && parserState->jsonStr[parserState->jsonStrPos] != c)) {
		json_error_lineno(err, parserState);
		return false;
	}
	return true;
}

//Returns true if parserState contains the given state, false otherwise
static inline int json_parser_check_state(json_parser_state* parserState, int state) {
	return parserState->state & (1 << state);
}
//Adds state to parserState and returns new state
static inline int json_parser_add_state(json_parser_state* parserState, int state) {
	return parserState->state |= (1 << state);
}
//Remove state from parserState and returns new state
static inline int json_parser_remove_state(json_parser_state* parserState, int state) {
	return parserState->state &= ~(1 << state);
}

//Returns a string representing the parser state
const char* json_parser_get_state_string(json_parser_state* parserState) {
	if (parserState) {
		return JSON_PARSER_STATE_NAMES[parserState->state];
	}
	return "";
}


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_PARSER_C
