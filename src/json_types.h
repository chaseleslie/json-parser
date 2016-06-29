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


/**
 *  @file json_types.h
 *  @brief JSON parser library type definitions and functions
 *
 *  This header declares a number of types and functions used by the
 *  library.
 */


#ifndef JSON_TYPES_H
#define JSON_TYPES_H


#ifndef JSON_TOP_LVL
#error "The file json_types.h must not be included directly. Include 'json.h' instead."
#endif	//#ifndef JSON_TOP_LVL


#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


/**
 *  @brief Enum representing the type of JSON value
 */
typedef enum JSON_VALUE {
	unspecified_value = 0,
	string_value,
	number_value,
	object_value,
	array_value,
	true_value,
	false_value,
	null_value,
	JSON_VALUE_COUNT
} JSON_VALUE;

/**
 *  @brief Table of strings to lookup name of JSON values
 */
extern const char* const JSON_VALUE_NAMES[];

/**
 *  @brief Enum representing state of parsing
 */
typedef enum JSON_PARSER_STATE {
	init_state = 0,
	complete_state,
	error_state,
	JSON_PARSER_STATE_MAX
} JSON_PARSER_STATE;

/**
 *  @brief Table of strings to lookup name of parsing state
 */
extern const char* const JSON_PARSER_STATE_NAMES[];

/**
 *  @brief Enum representing tokens in JSON text
 */
typedef enum JSON_TOKEN {
	json_token_quote = 0,
	json_token_plus,
	json_token_minus,
	json_token_lbrace,
	json_token_rbrace,
	json_token_lbrack,
	json_token_rbrack,
	json_token_colon,
	json_token_comma,
	json_token_backslash,
	JSON_TOKEN_COUNT
} JSON_TOKEN;

/**
 *  @brief Table of strings to lookup name of token
 */
extern const char JSON_TOKEN_NAMES[];

/**
 *  @brief JSON parser options
 *
 *  Types to set parser options with calls to json_parser_setopt(). Each
 *  opt description is followed by the expected C type and default value
 *  in parenthesis.
 */
typedef enum JSON_PARSER_OPT {
	/*! The max nested depth allowed in JSON text source; int (128) */
	json_max_nested_level = 0,
	/*! The stream to write error messages to or NULL; FILE* (stderr) */
	json_error_stream,
	JSON_PARSER_OPT_MAX
} JSON_PARSER_OPT;

extern const char* const JSON_EMPTY_STRING;

extern size_t align_offset(size_t offset, size_t align);

struct json_parser_state;
typedef struct json_parser_state json_parser_state;

struct json_allocator;
/*! Typedef for json_allocator struct */
typedef struct json_allocator json_allocator;
struct json_factory;
/*! Typedef for json_factory struct */
typedef struct json_factory json_factory;

struct json_object;
/*! Typedef for json_object struct */
typedef struct json_object json_object;
struct json_value;
/*! Typedef for json_value struct */
typedef struct json_value json_value;

struct json_string;
/*! Typedef for json_string struct */
typedef struct json_string json_string;
struct json_number;
/*! Typedef for json_number struct */
typedef struct json_number json_number;
struct json_array;
/*! Typedef for json_array struct */
typedef struct json_array json_array;
struct json_true;
/*! Typedef for json_true struct */
typedef struct json_true json_true;
struct json_false;
/*! Typedef for json_false struct */
typedef struct json_false json_false;
struct json_null;
/*! Typedef for json_null struct */
typedef struct json_null json_null;


/*! Typedef for a malloc-like alloc function */
typedef void* (*alloc_function)(size_t);
/*! Typedef for a free-like dealloc function */
typedef void (*free_function)(void*);

/**
 *  @brief Initialize the types for this parser instance
 *
 *  This function is called by json_parser_init() to create and initialize a
 *  json_allocator and json_factory, setting the appropriate functions for
 *  each.
 *
 *  @param[in] allocFunction Function pointer to an alloc function
 *  @param[in] freeFunction Function pointer to a dealloc function
 *  @param[out] jsAllocOut Pointer to created json_allocator for this parser instance
 *  @param[out] jsFactoryOut Pointer to created json_factory for this parser instance
 *  @return Zero on success, nonzero on failure
 */
int json_types_init(alloc_function allocFunction, free_function freeFunction, json_allocator** jsAllocOut, json_factory** jsFactoryOut);

/*@{ */
/*! Returns a new json_object */
json_object* json_factory_new_json_object(json_factory* jsonFact, json_value* objParentValue);
/*! Returns a new json_value */
json_value* json_factory_new_json_value(json_factory* jsonFact, JSON_VALUE valValueType, void* valValue, JSON_VALUE valParentValueType, void* valParentValue);
/*! Returns a new json_string */
json_string* json_factory_new_json_string(json_factory* jsonFact, const char* strValue, size_t strValueLen, json_value* strParentValue);
/*! Returns a new json_number */
json_number* json_factory_new_json_number(json_factory* jsonFact, double numValue, json_value* numParentValue);
/*! Returns a new json_array */
json_array* json_factory_new_json_array(json_factory* jsonFact, json_value* arrParentValue);
/*! Returns a new json_true */
json_true* json_factory_new_json_true(json_factory* jsonFact, json_value* truParentValue);
/*! Returns a new json_false */
json_false* json_factory_new_json_false(json_factory* jsonFact, json_value* falParentValue);
/*! Returns a new json_null */
json_null* json_factory_new_json_null(json_factory* jsonFact, json_value* nulParentValue);
/*@} */

int json_object_add_pair(json_factory* jsonFact, json_object* obj, json_string* name, json_value* value);
int json_array_add_element(json_factory* jsonFact, json_array* arr, json_value* value);

const char* json_value_get_type(json_value* value);

/*@{ */
int json_visitor_free_all(json_parser_state* parserState, json_value* topVal);
int json_visitor_free_value(json_factory* jsonFact, json_value* value);
int json_visitor_free_object(json_factory* jsonFact, json_object* obj);
int json_visitor_free_array(json_factory* jsonFact, json_array* arr);
int json_visitor_free_string(json_factory* jsonFact, json_string* str);
int json_visitor_free_number(json_factory* jsonFact, json_number* num);
int json_visitor_free_true(json_factory* jsonFact, json_true* tru);
int json_visitor_free_false(json_factory* jsonFact, json_false* fals);
int json_visitor_free_null(json_factory* jsonFact, json_null* nul);
/*@} */


/**
 *  @brief Struct representing an allocator
 */
typedef struct json_allocator {
	/*! A malloc-like allocator function */
	alloc_function malloc;
	/*! A free-like dealloc function */
	free_function free;
} json_allocator;

/**
 *  @brief Struct representing a factory to make new objects
 */
typedef struct json_factory {
	/*@{ */
	/*! A pointer to the json_allocator object */
	json_allocator* allocator;
	/*@} */
	
	/*@{ */
	/*! Returns a new json_object */
	json_object* (*new_json_object)(json_factory* jsonFact, json_value* objParentValue);
	/*! Returns a new json_value */
	json_value* (*new_json_value)(json_factory* jsonFact, JSON_VALUE valValueType, void* valValue, JSON_VALUE valParentValueType, void* valParentValue);
	/*! Returns a new json_string */
	json_string* (*new_json_string)(json_factory* jsonFact, const char* strValue, size_t strValueLen, json_value* strParentValue);
	/*! Returns a new json_number */
	json_number* (*new_json_number)(json_factory* jsonFact, double numValue, json_value* numParentValue);
	/*! Returns a new json_array */
	json_array* (*new_json_array)(json_factory* jsonFact, json_value* arrParentValue);
	/*! Returns a new json_true */
	json_true* (*new_json_true)(json_factory* jsonFact, json_value* truParentValue);
	/*! Returns a new json_false */
	json_false* (*new_json_false)(json_factory* jsonFact, json_value* falParentValue);
	/*! Returns a new json_null */
	json_null* (*new_json_null)(json_factory* jsonFact, json_value* nulParentValue);
	/*@} */
} json_factory;


/**
 *  @brief Struct representing a JSON object
 *
 *  A JSON object represents pairs of names and values.
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-4
 */
typedef struct json_object {
	//TODO: Have json_object contain json_string in a json_value ??
	/*@{ */
	/*! Array of property names */
	json_string** names;
	/*! Array of property values */
	json_value** values;
	/*! Current number of names and values occupied */
	size_t size;
	/*! Capacity of names and values */
	size_t capacity;
	/*@} */
	
	/*@{ */
	/*! Pointer to the json_value object containing this object */
	json_value* parentValue;
	/*@} */
} json_object;

/**
 *  @brief Struct representing a JSON value
 *
 *  A JSON value can be an object, array, number, string or a literal value
 *  of @c false, @c null or @c true.
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-3
 */
typedef struct json_value {
	/*@{ */
	/*! Type of JSON value this object contains */
	JSON_VALUE valueType;
	/*! Pointer to the value object */
	void* value;
	/*@} */
	
	/*@{ */
	/*! Type of object that contains this value (object_value, array_value or unspecified_value for toplevel) */
	JSON_VALUE parentValueType;
	/*! Pointer to the object/array that contains this value or NULL for toplevel */
	void* parentValue;
	/*@} */
} json_value;

/**
 *  @brief Struct representing a JSON string
 *
 *  A JSON string consists of text between double quotation marks. See
 *  RFC 7159 for more.
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-7
 */
typedef struct json_string {
	/*@{ */
	/*! Pointer to string representing UTF-8 encoded text of JSON string */
	const char* value;
	/*! Length of @p value */
	size_t valueLen;
	/*@} */
	
	/*@{ */
	/*! Pointer to the json_value object containing this string */
	json_value* parentValue;
	/*@} */
} json_string;

/**
 *  @brief Struct representing a JSON number
 *
 *  A JSON number consists of decimal digits in base 10 followed by an optional
 *  fractional part and an optional exponent part.
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-6
 */
typedef struct json_number {
	/*@{ */
	/*! The value of the JSON number */
	double value;
	/*@} */
	
	/*@{ */
	/*! Pointer to the json_value object containing this number */
	json_value* parentValue;
	/*@} */
} json_number;

/**
 *  @brief Struct representing a JSON array
 *
 *  A JSON array consists of zero or more JSON values surrounded by square
 *  brackets @c [].
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-5
 */
typedef struct json_array {
	/*@{ */
	/*! Array of values */
	json_value** values;
	/*! Current number of values occupied */
	size_t size;
	/*! Capacity of names and values */
	size_t capacity;
	/*@} */
	
	/*@{ */
	/*! Pointer to the json_value object containing this array */
	json_value* parentValue;
	/*@} */
} json_array;

/**
 *  @brief Struct representing a JSON literal @c true
 *
 *  A JSON literal @c true value.
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-3
 */
typedef struct json_true {
	/*! Pointer to the json_value object containing this literal */
	json_value* parentValue;
} json_true;

/**
 *  @brief Struct representing a JSON literal @c false
 *
 *  A JSON literal @c false value.
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-3
 */
typedef struct json_false {
	/*! Pointer to the json_value object containing this literal */
	json_value* parentValue;
} json_false;

/**
 *  @brief Struct representing a JSON literal @c null
 *
 *  A JSON literal @c null value.
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-3
 */
typedef struct json_null {
	/*! Pointer to the json_value object containing this literal */
	json_value* parentValue;
} json_null;


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_TYPES_H
