#ifndef JSON_PARSER_H
#define JSON_PARSER_H


#ifndef JSON_TOP_LVL
#error "The file json_parser.h must not be included directly. Include 'json.h' instead."
#endif	//#ifndef JSON_TOP_LVL


#include "json_types.h"


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


typedef struct json_parser_state {
	const char* jsonStr;
	size_t jsonStrLength;
	size_t jsonStrPos;
	int state;
	size_t nestedLevel;
	size_t maxNestedLevel;
	json_allocator* JSON_Allocator;
	json_factory* JSON_Factory;
} json_parser_state;


json_parser_state* json_parser_init(void* (*allocFunction)(size_t), void (*freeFunction)(void*));

int json_parser_clear(json_parser_state* parserState);

json_value* json_parser_parse(json_parser_state* parserState, const char* jsonStr, size_t jsonStrLength);

json_value* json_parser_parse_value(json_parser_state* parserState, void* parentValue, JSON_VALUE parentValueType);

json_object* json_parser_parse_object(json_parser_state* parserState, json_value* parentValue);

json_array* json_parser_parse_array(json_parser_state* parserState, json_value* parentValue);

json_number* json_parser_parse_number(json_parser_state* parserState, json_value* parentValue);

json_string* json_parser_parse_string(json_parser_state* parserState, json_value* parentValue);

json_true* json_parser_parse_true(json_parser_state* parserState, json_value* parentValue);

json_false* json_parser_parse_false(json_parser_state* parserState, json_value* parentValue);

json_null* json_parser_parse_null(json_parser_state* parserState, json_value* parentValue);

const char* json_parser_get_state_string(json_parser_state* parserState);


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_PARSER_H
