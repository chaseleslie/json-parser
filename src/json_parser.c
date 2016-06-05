#ifndef JSON_PARSER_CC
#define JSON_PARSER_CC


#define JSON_TOP_LVL 1

#include "json_parser.h"
#include "json_types.h"
#include "json_utils.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


const size_t JSON_MAX_NESTED_DEFAULT = 128;

static void json_parser_skip_ws(json_parser_state* parserState);

static inline int json_parser_check_state(json_parser_state* parserState, int state);
static inline int json_parser_add_state(json_parser_state* parserState, int state);
static inline int json_parser_remove_state(json_parser_state* parserState, int state);


//TODO: Accept setting for max nested level from user/caller; enforce max nesting level
json_parser_state* json_parser_init(void* (*allocFunction)(size_t), void (*freeFunction)(void*)) {
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
	
	return parserState;
}

int json_parser_clear(json_parser_state* parserState) {
	if (!parserState) {
		return 1;
	}
	
	void (*freeFunction)(void*) = parserState->JSON_Allocator->free;
	freeFunction(parserState->JSON_Factory);
	freeFunction(parserState->JSON_Allocator);
	freeFunction(parserState);
	
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
	
	switch (parserState->jsonStr[parserState->jsonStrPos]) {
		case '{': {
			parserState->jsonStrPos += 1;
			parserState->nestedLevel += 1;
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
			parserState->nestedLevel += 1;
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
		default: {
			if (strstr(parserState->jsonStr + parserState->jsonStrPos, JSON_VALUE_NAMES[true_value]) == (parserState->jsonStr + parserState->jsonStrPos)) {
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
			} else if (strstr(parserState->jsonStr + parserState->jsonStrPos, JSON_VALUE_NAMES[false_value]) == (parserState->jsonStr + parserState->jsonStrPos)) {
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
			} else if (strstr(parserState->jsonStr + parserState->jsonStrPos, JSON_VALUE_NAMES[null_value]) == (parserState->jsonStr + parserState->jsonStrPos)) {
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
				//TODO: handle error case
			}
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
		if (parserState->jsonStr[parserState->jsonStrPos] == JSON_TOKEN_NAMES[json_token_quote]) {
			parserState->jsonStrPos += 1;
			
			json_string* str = json_parser_parse_string(parserState, NULL);
			if (!str) {
				return obj;
			}
			
			json_parser_skip_ws(parserState);
			if (parserState->jsonStr[parserState->jsonStrPos] != JSON_TOKEN_NAMES[json_token_colon]) {
				json_error_lineno("json_parser:%u:%u Expecting ':'\n", parserState);
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
		} else {
			json_error_lineno("json_parser:%u:%u Expecting '\"'\n", parserState);
			json_parser_add_state(parserState, error_state);
			return obj;
		}
	} while (
		parserState->jsonStrPos < parserState->jsonStrLength
		&& parserState->jsonStr[parserState->jsonStrPos] == JSON_TOKEN_NAMES[json_token_comma]
		&& parserState->jsonStrPos++
	);
	
	json_parser_skip_ws(parserState);
	if (parserState->jsonStr[parserState->jsonStrPos] != JSON_TOKEN_NAMES[json_token_rbrace]) {
		json_error_lineno("json_parser:%u:%u Expected '}'\n", parserState);
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
			json_error_lineno("json_parser:%u:%u Expecting number\n", parserState);
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
	if (parserState->jsonStr[parserState->jsonStrPos] != JSON_TOKEN_NAMES[json_token_rbrack]) {
		json_error_lineno("json_parser:%u:%u Expected ']'\n", parserState);
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
		}
		parserState->jsonStrPos += 1;
	}
	
	if (!foundEndQuote) {
		json_error_lineno("json_parser:%u:%u Expecting '\"', reached eos\n", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	size_t dataLen = parserState->jsonStrPos - startPos;
	int ret;
	char* data = json_utils_unescape_string(parserState, parserState->jsonStr, dataLen, &ret);
	if (!data || ret) {
		json_error_lineno("json_parser:%u:%u Error: json_utils_unescape_string()", parserState);
		json_parser_add_state(parserState, error_state);
		return NULL;
	}
	
	str = parserState->JSON_Factory->new_json_string(parserState->JSON_Factory, data, parentValue);
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
	return NULL;
}


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_PARSER_CC
