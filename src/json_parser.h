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
 *  @file json_parser.h
 *  @brief JSON parser library parser types and functions
 *
 *  This header declares objects and functions to parse JSON text.
 */


#ifndef JSON_PARSER_H
#define JSON_PARSER_H


#ifndef JSON_TOP_LVL
#error "The file json_parser.h must not be included directly. Include 'json.h' instead."
#endif	//#ifndef JSON_TOP_LVL


#include "json_types.h"
#include <stdarg.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


/**
 *  @brief Struct representing the parser instance
 *
 *  Users shouldn't need to modify this directly, but access it through library calls.
 */
typedef struct json_parser_state {
	/*@{ */
	/*! Pointer to the JSON text string supplied by user, not modified */
	const char* jsonStr;
	/*! Length of the JSON text string */
	size_t jsonStrLength;
	/*! Current position in the JSON text string */
	size_t jsonStrPos;
	/*! State of the parser instance; see JSON_PARSER_STATE */
	int state;
	/*! Current nested level */
	size_t nestedLevel;
	/*! Max nested level */
	size_t maxNestedLevel;
	/*! Error stream */
	FILE* errorStream;
	/*@} */

	/*@{ */
	/*! Pointer to this parser's json_allocator */
	json_allocator* JSON_Allocator;
	/*! Pointer to this parser's json_factory */
	json_factory* JSON_Factory;
	/*@} */
} json_parser_state;


/*@{ */
/**
 *  @brief Set options for the JSON parser isntance
 *
 *  This function sets options for the JSON parser. See JSON_PARSER_OPT for
 *  the available options and what type of arguments they expect.
 *
 *  @param parserState Pointer to a previously initialized json_parser_state
 *  @param opt Option to set
 *  @return Zero on success, or nonzero on failure
 *
 *  @see JSON_PARSER_OPT
 */
int json_parser_setopt(json_parser_state* parserState, JSON_PARSER_OPT opt, ...);

/**
 *  @brief Initialize the parser instance
 *
 *  This function is called first to initialize the JSON parser instance. It takes two
 *  arguments, @p allocFunction which is a malloc-like alloc function (see the
 *  prototype of alloc_function), and @p freeFunction which is a free-like dealloc
 *  function (see free_function).
 *
 *  @c NULL can be given as both arguments, in which case the library will default
 *  to using malloc and free internally. If a valid function pointer is given for
 *  @p allocFunction and @c NULL is given as @p freeFunction, the library will
 *  allocate memory from @p allocFunction and use a noop free function to free
 *  memory. This is useful if you are allocating memory from a memory pool.
 *  Passing @c NULL for @p allocFunction and a valid function pointer for
 *  @p freeFunction, the library will default to using malloc and free.
 *
 *  A pointer to the json_parser_state is returned on success, or @c NULL on failure.
 *  The returned pointer is used in subsequent calls to the library. The returned
 *  json_parser_state should be deallocated by calling json_parser_clear() when the
 *  user is finished with the library.
 *
 *  @param allocFunction Pointer to a malloc-like function to allocate memory from, or NULL
 *  @param freeFunction Pointer to a  free-like function to deallocate memory from, or NULL
 *  @return Pointer to a json_parser_state for this parser instance, or NULL on failure
 *
 *  @see alloc_function free_function json_parser_clear() json_parser_state
 */
json_parser_state* json_parser_init(alloc_function allocFunction, free_function freeFunction);

/**
 *  @brief Clear the json_parser_state and free its memory
 *
 *  This function destroys the passed json_parser_state and frees the memory that
 *  was allocated for it. It should be called after the user is finished with the library.
 *
 *  @param parserState Pointer to the json_parser_state to clear
 *  @return Zero on success, or nonzero on failure
 *
 *  @see json_parser_init() json_parser_state
 */
int json_parser_clear(json_parser_state* parserState);

/**
 *  @brief Reset the json_parser_state to prepare for parsing again
 *
 *  This function resets the json_parser_state pointed to by @p parserState to
 *  prepare for additional calls to json_parser_parse(). This function does not
 *  completely deallocate the parser state instance, for that see json_parser_clear().
 *
 *  @param parserState Pointer to the json_parser_state to reset
 *  @return Zero on success, or nonzero on failure
 *
 *  @see json_parser_init() json_parser_state json_parser_clear()
 */
int json_parser_reset(json_parser_state* parserState);
/*@} */

/*@{ */
/**
 *  @brief Parse the passed JSON text into values
 *
 *  This function is the entrypoint to parsing a JSON text and must be called after
 *  a json_parser_state is initialized with json_parser_init().
 *
 *  The passed JSON text source string need not be @c NULL terminated but the
 *  argument @p jsonStrLength must contain the actual length of the string in
 *  bytes. @p jsonStr is assumed to be UTF-8 encoded or ASCII. UTF-8 encoded
 *  JSON is recommended by RFC 7159 to increase interoperability of JSON.
 *
 *  @param parserState Pointer to instance of parser state created with json_parser_init()
 *  @param jsonStr A string containing the JSON text to parse
 *  @param jsonStrLength Length of the JSON text in @p jsonStr
 *  @return The top-level JSON value parsed from the JSON text, or NULL on failure
 *
 *  @see json_value json_parser_state https://tools.ietf.org/html/rfc7159#section-8.1
 */
json_value* json_parser_parse(json_parser_state* parserState, const char* jsonStr, size_t jsonStrLength);

json_value* json_parser_parse_value(json_parser_state* parserState, void* parentValue, JSON_VALUE parentValueType);

json_object* json_parser_parse_object(json_parser_state* parserState, json_value* parentValue);

json_array* json_parser_parse_array(json_parser_state* parserState, json_value* parentValue);

json_number* json_parser_parse_number(json_parser_state* parserState, json_value* parentValue);

json_string* json_parser_parse_string(json_parser_state* parserState, json_value* parentValue);

json_true* json_parser_parse_true(json_parser_state* parserState, json_value* parentValue);

json_false* json_parser_parse_false(json_parser_state* parserState, json_value* parentValue);

json_null* json_parser_parse_null(json_parser_state* parserState, json_value* parentValue);
/*@} */

/**
 *  @brief Get a C-string representation of the parser state
 *
 *  @param parserState Pointer to parser state instance
 *  @return A C-string representing the state of the parser
 *
 *  @see JSON_PARSER_STATE
 */
const char* json_parser_get_state_string(json_parser_state* parserState);


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_PARSER_H
