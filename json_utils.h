#ifndef JSON_UTILS_H
#define JSON_UTILS_H


#ifndef JSON_TOP_LVL
#error "The file json_utils.h must not be included directly. Include 'json.h' instead."
#endif	//#ifndef JSON_TOP_LVL


#include "json_parser.h"
#include "json_types.h"

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


void json_error(const char* err);

void json_error_lineno(const char* err, json_parser_state* parserState);

char* json_utils_unescape_string(json_parser_state* parserState, const char* str, size_t n, int* state);


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_UTILS_H
