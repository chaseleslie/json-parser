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
 *  @file json_introspect.h
 *  @brief JSON parser library functions for introspection
 *
 *  This header declares a number of functions to manipulate JSON
 *  values.
 */


#ifndef JSON_INTROSPECT_H
#define JSON_INTROSPECT_H


#ifndef JSON_TOP_LVL
#error "The file json_types.h must not be included directly. Include 'json.h' instead."
#endif	//#ifndef JSON_TOP_LVL


#include "json_types.h"


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


/*! Prototype for a callback function used with json_object_foreach() */
typedef int (*json_object_foreach_cb)(json_object*, json_string*, json_value*);
/*! Prototype for a callback function used with json_array_foreach() */
typedef int (*json_array_foreach_cb)(json_array*, json_value*);

/*@{ */
/**
 *  @brief Iterate over all the name/value pairs in an object
 *
 *  This function allows the caller to supply a callback function that will receive the
 *  json_string and json_value of each name/value pair in an object. The callback should
 *  match the prototype for json_object_foreach_cb and return nonzero to keep iterating
 *  or zero to stop.
 *
 *  @param[in] val Pointer to json_value containing the json_object to iterate over
 *  @param[in] iter Callback function to iterate over object
 *  @return Zero on success, nonzero on failure
 *
 *  @see json_object_foreach_cb json_object_foreach_obj()
 */
int json_object_foreach(json_value* val, json_object_foreach_cb iter);
/**
 *  @brief Iterate over all the name/value pairs in an object
 *
 *  This function behaves similarly to json_object_foreach(), taking a pointer to a
 *  json_object directly instead of it's containing json_value.
 *
 *  @param[in] obj Pointer to json_object to iterate over
 *  @param[in] iter Callback function to iterate over object
 *  @return Zero on success, nonzero on failure
 *
 *  @see json_object_foreach_cb json_object_foreach()
 */
int json_object_foreach_obj(json_object* obj, json_object_foreach_cb iter);
/**
 *  @brief Iterate over all the values in an array
 *
 *  This function allows the caller to supply a callback function that will receive the
 *  json_value of each value in an array. The callback should match the prototype for
 *  json_array_foreach_cb and return nonzero to keep iterating or zero to stop.
 *
 *  @param[in] val Pointer to json_value containing the json_array to iterate over
 *  @param[in] iter Callback function to iterate over array
 *  @return Zero on success, nonzero on failure
 *
 *  @see json_array_foreach_cb json_array_foreach_arr()
 */
int json_array_foreach(json_value* val, json_array_foreach_cb iter);
/**
 *  @brief Iterate over all the values in an array
 *
 *  This function behaves similarly to json_array_foreach(), taking a pointer to a
 *  json_array directly instead of it's containing json_value.
 *
 *  @param[in] arr Pointer to json_array to iterate over
 *  @param[in] iter Callback function to iterate over array
 *  @return Zero on success, nonzero on failure
 *
 *  @see json_array_foreach_cb json_array_foreach()
 */
int json_array_foreach_arr(json_array* arr, json_array_foreach_cb iter);
/*@} */

/**
 *  @brief Query a JSON value with JSON Pointer
 *
 *  This function uses JSON Pointer syntax to query a JSON value. The
 *  syntax of JSON pointer is defined in RFC 6901. JSON Pointers can
 *  query a value that is a child of an object or array. @c NULL will be
 *  returned if @p value doesn't reference a json_object or json_array,
 *  if there is an error or if the referenced value is not found. Otherwise,
 *  a pointer to the referenced json_value is returned.
 *
 *  Since JSON Pointer can contain reference tokens that consist of
 *  control characters, the length of the query string must be passed
 *  to accomodate the null Unicode character.
 *
 *  @param parserState A pointer to the parser isntance
 *  @param value A pointer to the json_value to query from
 *  @param query A string containing the query
 *  @param queryLen Length of the query string @p query
 *  @return A pointer to the referenced json_value, or @c NULL on failure
 *
 *  @see https://tools.ietf.org/html/rfc6901
 */
 json_value* json_value_query(
	json_parser_state* parserState,
	json_value* value,
	const char* query,
	const size_t queryLen
);

/**
 *  @brief Enum of flags to control JSON serialization
 */
enum JSON_STRINGIFY_FLAGS {
	/*! Default flags */
	json_stringify_default = 0,
	/*! Add a space after ':' and ',' tokens */
	json_stringify_spaces = 1,
	/*! Use newlines and indentation */
	json_stringify_indent = 2,
	/*! Escape chars outside of Unicode BMP to \\uXXXX\\uXXXX UTF-16 surrogate pairs */
	json_stringify_escape_non_bmp = 4,
	/*! Escape all chars outside ASCII to \\uXXXX, implies json_stringify_escape_non_bmp  */
	json_stringify_escape_non_ascii = 8
};

/**
 *  @brief Stringify a JSON value into a UTF-8 encoded string
 *
 *  This function constructs a string representation of the passed
 *  JSON value @p value. Memory for the string is allocated from
 *  the allocator given when the passed @p parserState was
 *  initialized. It must be <b>freed manually</b> by the free-like function
 *  passed to json_parser_init(), or with @c free() if default alloc
 *  is used.
 *
 *  The returned string is UTF-8 encoded.
 *
 *  @param parserState A pointer to the parser instance
 *  @param value A pointer to the json_value to stringify
 *  @param indent A C string with the chars to use for indentation, or @c NULL for default
 *  @param flags A bitmask of 
 *  @param[out] strLen A pointer to a @c size_t to receive the length of the returned string
 *  @return A UTF-8 encoded string containing the stringified JSON value
 */
 char* json_value_stringify(
	json_parser_state* parserState,
	json_value* value,
	const char* indent,
	int flags,
	size_t* strLen
);


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_INTROSPECT_H